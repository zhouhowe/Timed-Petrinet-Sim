
#include "event.h"

Event::Event(double enabled_time, Transition transition)
{
	this->time = enabled_time;
	this->transition = transition;
}


bool CompareEvent::operator()(Event& e1, Event& e2) {
	if(e1.time > e2.time) return true;
	else return false;
}
