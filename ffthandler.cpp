#include "FFTHandler.h"

FFTHandler::FFTHandler() : vReal(nullptr), vImag(nullptr), rfFeatures(nullptr), FFT(nullptr), initialized(false) {
    // 建構子
}

FFTHandler::~FFTHandler() {
    // 析構子，確保資源釋放
    cleanup();
}

bool FFTHandler::initialize() {
    // 記憶體初始化
    vReal = new (std::nothrow) double[FFT_SIZE];
    vImag = new (std::nothrow) double[FFT_SIZE];
    rfFeatures = new (std::nothrow) float[FEATURE_SIZE];
    
    if (!vReal || !vImag || !rfFeatures) {
        cleanup();
        return false;
    }
    
    FFT = new (std::nothrow) ArduinoFFT<double>(vReal, vImag, FFT_SIZE, SAMPLE_RATE);
    if (!FFT) {
        cleanup();
        return false;
    }
    
    initialized = true;
    return true;
}

void FFTHandler::cleanup() {
    if (vReal) {
        delete[] vReal;
        vReal = nullptr;
    }
    if (vImag) {
        delete[] vImag;
        vImag = nullptr;
    }
    if (rfFeatures) {
        delete[] rfFeatures;
        rfFeatures = nullptr;
    }
    if (FFT) {
        delete FFT;
        FFT = nullptr;
    }
    initialized = false;
}

bool FFTHandler::isInitialized() const {
    return initialized;
}

void FFTHandler::readSamples(File &file, int fftSize, double *vReal, double *vImag, uint16_t audioFormat) {
    if (!vReal || !vImag) return;  // 防止空指標存取

    for (int i = 0; i < fftSize; i++) {
        if (file.available()) {
            if (audioFormat == 1) { // PCM 16-bit
                int16_t sample = 0;
                file.read(reinterpret_cast<uint8_t*>(&sample), sizeof(int16_t));
                vReal[i] = static_cast<double>(sample) / 32768.0;
                vImag[i] = 0.0;
            } else if (audioFormat == 3) { // IEEE 754 32-bit float
                float sample = 0.0f;
                if (file.read(reinterpret_cast<uint8_t*>(&sample), sizeof(float)) == sizeof(float)) {
                    if (isnan(sample) || !isfinite(sample)) {  // 避免 NaN 或 Inf 值
                        sample = 0.0f;
                    }
                    vReal[i] = static_cast<double>(sample);
                    vImag[i] = 0.0;
                } else {
                    Serial.println("Error reading float sample!");
                    vReal[i] = 0.0;
                    vImag[i] = 0.0;
                }
            } else {
                Serial.println("Unsupported audio format!");
                return;
            }
        } else {
            vReal[i] = 0.0;
            vImag[i] = 0.0;
        }
    }
}

bool FFTHandler::processAudioChunk(File &audioFile, uint16_t audioFormat) {
    if (!initialized) return false;
    
    if (!audioFile.available()) return false;
    
    // 讀取音頻樣本
    readSamples(audioFile, FFT_SIZE, vReal, vImag, audioFormat);

    // 執行FFT
    FFT->windowing(FFTWindow::Hamming, FFTDirection::Forward);
    FFT->compute(FFTDirection::Forward);
    FFT->complexToMagnitude();
    
    // 將FFT結果轉換為隨機森林特徵
    saveFeaturesToArray();
    
    return true;
}

void FFTHandler::saveFeaturesToArray() {
    // 將double类型的FFT結果轉換成float类型給隨機森林模型使用
    for (int i = 0; i < FEATURE_SIZE; i++) {
        rfFeatures[i] = static_cast<float>(vReal[i]);
    }
}

void FFTHandler::saveFeaturesToFile(File &featureFile) {
    if (!initialized || !vReal) return;

    for (int i = 0; i < FEATURE_SIZE; i++) {
        char formattedValue[10];
        dtostrf(vReal[i], 1, 6, formattedValue);  // 保留6位小數
        featureFile.print(formattedValue);
        if (i < FEATURE_SIZE - 1) featureFile.print(", ");
    }
    featureFile.println();
}

float* FFTHandler::getFeatures() {
    return rfFeatures;
}
