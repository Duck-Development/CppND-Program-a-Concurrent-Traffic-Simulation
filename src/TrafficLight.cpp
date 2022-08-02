#include <iostream>
#include <random>
#include "TrafficLight.h"

using namespace std::chrono_literals;


/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    T ret;
    std::unique_lock<std::mutex> uni(_mtx);
    _cnd.wait(uni, [this] { return !_que.empty(); }); // pass unique lock to condition variable
    ret = _que.front();
    _que.pop_front();
    return ret;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> guard(_mtx);
    _que.emplace_back(msg);
    _cnd.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
    std::srand(std::time(0)); // init random generaor 
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(_massageQ.receive() != TrafficLightPhase::green);

}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 

    // launch drive function in a thread
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

std::chrono::seconds getPhaseTime(){
    return std::chrono::seconds(4+(std::rand()%3));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{

    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    
    auto lastPhaseChage = std::chrono::system_clock::now();
    auto waitTime = getPhaseTime();
    while(true)
    {
        auto timeDiff = std::chrono::system_clock::now() - lastPhaseChage;
        if (timeDiff > waitTime)
        {
            lastPhaseChage = std::chrono::system_clock::now();
            waitTime = getPhaseTime();
            
            if (_currentPhase == TrafficLightPhase::green)
            {
                _currentPhase = TrafficLightPhase::red;
                //std::cout << "from green to red" << std::endl;
            } 
            else 
            {
                _currentPhase = TrafficLightPhase::green;
                //std::cout << "from red to green" << std::endl;
            }
            auto tmp =_currentPhase;
            _massageQ.send(std::move(tmp));
            // emit to mqCD ..
        }
        std::this_thread::sleep_for(1ms);
    }
}