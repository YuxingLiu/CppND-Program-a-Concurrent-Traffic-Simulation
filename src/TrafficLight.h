#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

enum TrafficLightPhase
{
    red,
    green,
};

// message queue to send and receive TrafficLightPhase in a thread-safe manner
template <class T>
class MessageQueue
{
public:
    T receive();
    void send(T &&msg);
private:
    std::deque<T> _queue;
    std::condition_variable _cond;
    std::mutex _mutex;
};

class TrafficLight : public TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();

    // getters / setters
    TrafficLightPhase getCurrentPhase();

    // typical behaviour methods
    void waitForGreen();
    void simulate();

private:
    // typical behaviour methods
    void cycleThroughPhases();

    // private members
    TrafficLightPhase _currentPhase;
    std::condition_variable _condition;
    std::mutex _mutex;
    MessageQueue<TrafficLightPhase> _msgQueue;
};

#endif