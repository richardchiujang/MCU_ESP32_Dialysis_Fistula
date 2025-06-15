#ifndef PREDICTION_ENGINE_H
#define PREDICTION_ENGINE_H

#include <Arduino.h>
#include "random_forest_model.h"

class PredictionEngine {
public:
    PredictionEngine();
    ~PredictionEngine();
    
    bool initialize();
    int predict(float *features);
    
private:
    Eloquent::ML::Port::RandomForest *rf;
    bool initialized;
};

#endif // PREDICTION_ENGINE_H
