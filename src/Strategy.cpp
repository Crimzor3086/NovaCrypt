#include "Strategy.h"

Strategy::Strategy() : min_confidence_threshold_(0.7) {}

Strategy::Signal Strategy::generateSignal(const AIEngine::Decision& aiDecision) {
    Signal signal;
    signal.action = aiDecision.action;
    signal.confidence = aiDecision.confidence;
    
    // Only execute trades that meet our confidence threshold
    signal.should_execute = (signal.confidence >= min_confidence_threshold_);
    
    return signal;
} 