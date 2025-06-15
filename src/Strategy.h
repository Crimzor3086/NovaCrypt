#pragma once
#include <string>
#include "AIEngine.h"

class Strategy {
public:
    struct Signal {
        std::string action;
        double confidence;
        bool should_execute;
    };

    Strategy();
    Signal generateSignal(const AIEngine::Decision& aiDecision);
    
private:
    double min_confidence_threshold_;  // Minimum confidence required to execute a trade
}; 