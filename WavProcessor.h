#ifndef WAV_PROCESSOR_H
#define WAV_PROCESSOR_H

#include <Arduino.h>
#include <SD.h>
#include "FFTHandler.h"
#include "PredictionEngine.h"

class WavProcessor {
public:
    WavProcessor();
    ~WavProcessor();
    
    bool initialize();
    void cleanup();
    void processWavFilesInFolder(const char *folderPath, File &resultFile, PredictionEngine &predictionEngine);
    
private:
    FFTHandler fftHandler;
    File wavFile;
    uint16_t audioFormat;
    
    void readWavHeader(File &file, uint16_t &audioFormat);
    void performFFTAndPredict(File &wavFile, const String &folderPath, const String &fileName, 
                             File &resultFile, PredictionEngine &predictionEngine);
};

#endif // WAV_PROCESSOR_H