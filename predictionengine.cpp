#include "PredictionEngine.h"

PredictionEngine::PredictionEngine() : rf(nullptr), initialized(false) {
    // 建構子
}

PredictionEngine::~PredictionEngine() {
    // 析構子
    if (rf) {
        delete rf;
        rf = nullptr;
    }
    initialized = false;
}

bool PredictionEngine::initialize() {
    rf = new (std::nothrow) Eloquent::ML::Port::RandomForest();
    if (!rf) {
        return false;
    }
    initialized = true;
    return true;
}

int PredictionEngine::predict(float *features) {
    if (!initialized || !rf) {
        return 0;
    }
    return rf->predict(features);
}
