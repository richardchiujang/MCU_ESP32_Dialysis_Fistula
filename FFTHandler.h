#ifndef FFT_HANDLER_H
#define FFT_HANDLER_H

#include <Arduino.h>
#include <SD.h>
#include "arduinoFFT.h"
#include "esp_heap_caps.h"

#define SAMPLE_RATE 8000  // 取樣率
#define FFT_SIZE 512     // FFT 解析度，必須為2的次方
#define FEATURE_SIZE 128  // 取主要的128個頻域特徵

class FFTHandler {
public:
    FFTHandler();
    ~FFTHandler();
    
    bool initialize();
    void cleanup();
    bool isInitialized() const;
    bool processAudioChunk(File &audioFile, uint16_t audioFormat);
    void saveFeaturesToFile(File &featureFile);
    float* getFeatures();
    
private:
    double *vReal;
    double *vImag;
    float *rfFeatures;
    ArduinoFFT<double> *FFT;
    bool initialized;
    
    void readSamples(File &file, int fftSize, double *vReal, double *vImag, uint16_t audioFormat);
    void saveFeaturesToArray();
};

#endif // FFT_HANDLER_H