
#ifndef _PETRI_NET_H_
#define _PETRI_NET_H_

typedef long Transition;
typedef long Place;

void fire(Transition t);
void change_state(Transition t);

bool timing();

bool isFirable(Transition t);
bool isTerminated();

void init();

void read_matrix( const char* filename );
void read_initial_state( const char* filename );
void read_place_type( const char* filename );
void read_distribution( const char* filename);
void read_deadlock_state( const char* filename);
void read_prob(const char* filename);

bool isResourcePlace( Place p);
bool isIdlePlace(Place p);
bool isDeadlock();

Transition getOutputTransition( Place p);
Place getOutputPlace(Transition t);
double getDwellTime( Transition t);
bool schedule_event(Place p);

void print_marking();
void update_stat();
void print_stat(Place begin, Place end);

void cleanup();


#endif
