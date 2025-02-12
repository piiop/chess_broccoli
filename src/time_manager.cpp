#include "../include/time_manager.hpp"

namespace chess
{
    double TimeManager::calculateMoveTime(double remainingTime, int numLegalMoves, int gamePhase)
    {
        // With 0.1s Simple Delay and no time banking, we should always use close to our full delay
        // but leave a tiny safety margin for network/computation overhead
        return 0.095; // 95ms, leaving 5ms safety margin
    }
}