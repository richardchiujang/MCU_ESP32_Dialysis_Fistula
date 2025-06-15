#include "WavProcessor.h"

WavProcessor::WavProcessor() : audioFormat(0) {
    // 建構子
}

WavProcessor::~WavProcessor() {
    // 析構子，確保資源釋放
    cleanup();
}

bool WavProcessor::initialize() {
    return fftHandler.initialize();
}

void WavProcessor::cleanup() {
    fftHandler.cleanup();
}

void WavProcessor::processWavFilesInFolder(const char *folderPath, File &resultFile, PredictionEngine &predictionEngine) {
    File dir = SD.open(folderPath);
    if (!dir || !dir.isDirectory()) {
        Serial.print("Failed to open directory: ");
        Serial.println(folderPath);
        return;
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String fileName = file.name();
            if (fileName.endsWith(".wav")) {
                Serial.print("Processing file: ");
                Serial.println(String(folderPath) + '/' + fileName);
                
                // 打開WAV文件進行處理
                wavFile = SD.open(String(folderPath) + '/' + fileName, FILE_READ);
                if (!wavFile) {
                    Serial.println("Failed to open WAV file!");
                } else {
                    // 1. 讀取WAV頭部信息
                    readWavHeader(wavFile, audioFormat);
                    
                    // 2. 執行FFT並進行預測
                    performFFTAndPredict(wavFile, String(folderPath), fileName, resultFile, predictionEngine);
                    
                    wavFile.close();
                }
            }
        }
        file = dir.openNextFile();
    }
    
    dir.close();
}

void WavProcessor::readWavHeader(File &file, uint16_t &audioFormat) {
    if (file.size() < 44) {  
        Serial.println("Invalid WAV file! File too small.");
        file.close();
        return;
    }

    char chunkID[5] = {0};
    char format[5] = {0};
    char subchunk1ID[5] = {0};
    char subchunk2ID[5] = {0};
    uint32_t chunkSize, subchunk1Size, subchunk2Size;
    uint16_t numChannels, blockAlign, bitsPerSample;
    uint32_t sampleRate, byteRate;

    file.seek(0);
    file.readBytes(chunkID, 4);
    file.read(reinterpret_cast<uint8_t *>(&chunkSize), 4);
    file.readBytes(format, 4);

    file.readBytes(subchunk1ID, 4);
    file.read(reinterpret_cast<uint8_t *>(&subchunk1Size), 4);
    file.read(reinterpret_cast<uint8_t *>(&audioFormat), 2);
    file.read(reinterpret_cast<uint8_t *>(&numChannels), 2);
    file.read(reinterpret_cast<uint8_t *>(&sampleRate), 4);
    file.read(reinterpret_cast<uint8_t *>(&byteRate), 4);
    file.read(reinterpret_cast<uint8_t *>(&blockAlign), 2);
    file.read(reinterpret_cast<uint8_t *>(&bitsPerSample), 2);

    // 找到 "data" chunk
    while (file.available()) {
        file.readBytes(subchunk2ID, 4);
        file.read(reinterpret_cast<uint8_t *>(&subchunk2Size), 4);

        if (strncmp(subchunk2ID, "data", 4) == 0) {
            break;
        } else {
            // 確保不超過檔案大小
            uint32_t currentPos = file.position();
            if (currentPos + subchunk2Size >= file.size()) {
                Serial.println("Error: Invalid chunk size!");
                file.close();
                return;
            }

            file.seek(currentPos + subchunk2Size);  // 跳過此區塊
        }
    }

    Serial.println("========== WAV Header ==========");
    Serial.print("Audio Format: ");
    if (audioFormat == 1) {
        Serial.println("PCM (Pulse Code Modulation)");
    } else if (audioFormat == 3) {
        Serial.println("IEEE Float");
    } else {
        Serial.print("Other (Format Code: "); Serial.print(audioFormat); Serial.println(")");
    }
    Serial.print("Channels: "); Serial.println(numChannels);
    Serial.print("Sample Rate: "); Serial.println(sampleRate);
    Serial.print("Bits Per Sample: "); Serial.println(bitsPerSample);
    Serial.println("================================");

    if (strncmp(subchunk2ID, "data", 4) != 0) {
        Serial.println("Error: No 'data' chunk found!");
        file.close();
        return;
    }

    file.seek(file.position());  // 確保正確讀取音訊數據
}

void WavProcessor::performFFTAndPredict(File &wavFile, const String &folderPath, const String &fileName, 
                                       File &resultFile, PredictionEngine &predictionEngine) {
    if (!fftHandler.isInitialized()) {
        Serial.println("FFT Handler not initialized!");
        return;
    }

    // 同時保存FFT結果到txt文件
    String featureFileName = folderPath + '/' + fileName.substring(0, fileName.length() - 4) + ".txt";
    File featureFile = SD.open(featureFileName.c_str(), FILE_WRITE);
    if (!featureFile) {
        Serial.println("Failed to create feature file!");
        return;
    }

    int sampleCount = 0;
    int positiveCount = 0;

    while (wavFile.available()) {
        // 讀取音頻樣本並執行FFT
        if (fftHandler.processAudioChunk(wavFile, audioFormat)) {
            // 保存FFT結果到文本文件
            fftHandler.saveFeaturesToFile(featureFile);
            
            // 使用隨機森林模型進行預測
            int prediction = predictionEngine.predict(fftHandler.getFeatures());
            positiveCount += prediction;
            
            sampleCount++;
        } else {
            break;  // 處理錯誤或到達文件結尾
        }
    }

    featureFile.close();
    
    // 計算並保存預測結果
    float ratio = 0.0;
    if (sampleCount > 0) {
        ratio = static_cast<float>(positiveCount) / sampleCount;
    }
    
    // 輸出結果到Serial
    Serial.print("檔案: ");
    Serial.print(folderPath + '/' + fileName);
    Serial.print("  總處理筆數: ");
    Serial.print(sampleCount);
    Serial.print("  加總/總筆數: ");
    Serial.print(positiveCount);
    Serial.print("/");
    Serial.print(sampleCount);
    Serial.print(" = ");
    Serial.println(ratio, 4);
    
    // 寫入結果到result.txt
    resultFile.print("檔案: ");
    resultFile.print(folderPath + '/' + fileName);
    resultFile.print("  總處理筆數: ");
    resultFile.print(sampleCount);
    resultFile.print("  加總/總筆數: ");
    resultFile.print(positiveCount);
    resultFile.print("/");
    resultFile.print(sampleCount);
    resultFile.print(" = ");
    resultFile.println(ratio, 4);
}