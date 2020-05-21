#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // perform deque modification under the lock
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck, [this] { return !_queue.empty(); });    // _cond.wait() only blocks if returns false

    // remove last element from queue
    T t = std::move(_queue.back());
    _queue.pop_back();

    return t;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // perform deque modification under the lock
    std::lock_guard<std::mutex> lock(_mutex);

    // add a mew message to the queue
    _queue.push_back(std::move(msg));

    // notify client after sending a notification
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // repeatedly calls the receive function on the message queue
    while(true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if(_msgQueue.receive() == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // launch cycleThroughPhases function in a thread
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_real_distribution<double> distr(4000.0, 6000.0);   // cycle duration is a random value between 4 and 6 seconds

    // duration of a single simulation cycle in ms
    double cycleDuration = distr(eng);

    // init stop watch
    std::chrono::time_point<std::chrono::system_clock> lastUpdate = std::chrono::system_clock::now();
    while(true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if(timeSinceLastUpdate >= cycleDuration)
        {
            // toggle the phase of trafiic light
            if(_currentPhase == TrafficLightPhase::green)
                _currentPhase = TrafficLightPhase::red;
            else
                _currentPhase = TrafficLightPhase::green;

            // send the new TrafficLightPhase to message queue
            TrafficLightPhase msg = _currentPhase;
            _msgQueue.send(std::move(msg));

            // reset stop watch and random cycle duration for next cycle
            lastUpdate = std::chrono::system_clock::now();
            cycleDuration = distr(eng);
        }
    }
}