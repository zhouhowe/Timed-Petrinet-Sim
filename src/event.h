
#ifndef _EVENT_H_
#define _EVENT_H_
#include "petri-net.h"

class Event {
public:
	double time;		// time at which the event is firstly enabled
	Transition transition;	// the transitionneed to be fired

	Event();
	Event(double time, Transition transition);
};

class CompareEvent {
public:
	// return true if Event e1 occurs earlier than e2
	bool operator()(Event& e1, Event& e2);
};
#endif

