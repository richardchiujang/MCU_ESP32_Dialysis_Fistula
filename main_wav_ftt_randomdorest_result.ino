#include <Arduino.h>
#include <SD.h>
#include "WavProcessor.h"
#include "FFTHandler.h"
#include "PredictionEngine.h"

#define SD_CS 15  // SD卡模組的片選腳位

// 要處理的資料夾路徑
const char *folders[] = {
    "/vessel_data_test_FFT512/abnormal",
    "/vessel_data_test_FFT512/normal",
    "/record"
};

void setup() {
    Serial.begin(115200);
    delay(1000);  // 等待序列埠穩定
    
    Serial.println("Starting combined WAV FFT and prediction program...");

    if (!SD.begin(SD_CS)) {
        Serial.println("SD card initialization failed!");
        return;
    }
    Serial.println("SD card initialized.");

    // 初始化處理器
    WavProcessor wavProcessor;
    PredictionEngine predictionEngine;
    
    if (!wavProcessor.initialize() || !predictionEngine.initialize()) {
        Serial.println("Initialization failed! Restarting...");
        ESP.restart();
        return;
    }

    // 打開或創建結果文件
    File resultFile = SD.open("/result.txt", FILE_APPEND);
    if (!resultFile) {
        Serial.println("Failed to open or create result.txt!");
        wavProcessor.cleanup();
        return;
    }
    
    // 寫入標題
    resultFile.println("\n===== 新分析開始 =====");
    resultFile.print("日期時間: ");
    resultFile.println(String(__DATE__) + " " + String(__TIME__));
    
    // 處理所有資料夾內的 WAV 檔案
    for (const char *folder : folders) {
        Serial.print("Processing folder: ");
        Serial.println(folder);
        resultFile.println("\nFolder: " + String(folder));
        wavProcessor.processWavFilesInFolder(folder, resultFile, predictionEngine);
    }
    
    resultFile.println("===== 分析結束 =====\n");
    resultFile.close();
    
    Serial.println("All processing completed!");
    wavProcessor.cleanup();
}

void loop() {
    // 主迴圈不執行任何動作
    delay(10000);  // 暫停以避免CPU過度使用
}
