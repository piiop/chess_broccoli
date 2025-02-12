// time_manager.hpp
#ifndef TIME_MANAGER_HPP
#define TIME_MANAGER_HPP

#include <chrono>

namespace chess
{
    class TimeManager
    {
    public:
        TimeManager() = default;

        void startTimer()
        {
            startTime = std::chrono::steady_clock::now();
        }

        bool shouldStop() const
        {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - startTime);
            return elapsedTime >= maxTime;
        }

        void setTimeControls()
        {
            maxTime = std::chrono::milliseconds(95);
        }

    private:
        std::chrono::steady_clock::time_point startTime;
        std::chrono::milliseconds maxTime{95};

        // This remains for future expansion if needed
        double calculateMoveTime(double remainingTime, int numLegalMoves, int gamePhase);

        void setMoveTime(int timeMs)
        {
            maxTime = std::chrono::milliseconds(timeMs);
        }
    };
}
#endif