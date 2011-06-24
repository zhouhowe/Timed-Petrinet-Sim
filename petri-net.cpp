/*
	Petri net simulator, written by Hao Zhou
	haozhou@umich.edu on April 8, 2010
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <queue>
#include <vector>
#include <iterator>

#include "petri-net.h"
#include "event.h"
#include "VectorReader.hpp"
#include "lcgrand.h"

//#define _DEBUG_

using namespace std;

double sim_time;
double prev_sim_time=0;

int stream;

long n_place;
long n_transition;

long n_idle_place, n_operation_place, n_resource_place;
long n_deadlock_state;

vector<int> init_marking;
vector<int> deadlock_state;
vector<int> marking;	// current marking
vector<int> matrix;	// transition matrix
vector<double> mean_dwell_time;	// the mean dwell time of each transition 

vector<Place>* input_place;		// input place of a transition
vector<Place>* output_place;		// output place of a transition
vector<Transition>* output_transition;	// output transition of a place

vector<double>* output_tr_prob;		// output transition probability

vector<double> in_place_time;		// total amount of time that a token is in a resource place
					// used to calculate resource utilization

queue<Event> backlogged_event_list;

priority_queue<Event, vector<Event>, CompareEvent> future_event_list;


// advance simulation clock to the next event time
bool timing()
{
	if(future_event_list.size() == 0) return false;
	Event event = future_event_list.top();
	sim_time = event.time;
	#ifdef _DEBUG_
	cout<<endl<<"sim_time="<<sim_time<<endl;
	#endif
	
	// check the future event list, get the first event to fire
	fire( event.transition );
	
	// remove this event from the event list
	future_event_list.pop();
	return true;
}

void fire( Transition t )
{
	// get the incoming places
	// check for firability
	if( isFirable(t) )
	{

		#ifdef _DEBUG_
		cout<<"Fire transition "<<t<<" at time "<<sim_time<<endl;
		#endif

		// fire this transition, and change marking
		change_state(t);

		// schedule next event
		Place out_p = getOutputPlace(t);
		if( out_p != -1)
			schedule_event(out_p);
		
		// check backlogged event list
		#ifdef _DEBUG_
		cout<<"Backlogged Event List Size: "<<backlogged_event_list.size()<<endl;
		#endif
		for( int i=0; i < backlogged_event_list.size(); i++) {
			Event e = backlogged_event_list.front();
			double fire_time = e.time;
			Transition tr = e.transition;
			backlogged_event_list.pop();
			
			if(isFirable(tr)) {
				#ifdef _DEBUG_
				cout<<"Backlogged event fired: ("<<fire_time<<", "<<tr<<")"<<endl;
				#endif
				change_state(tr);

				Place out_p = getOutputPlace(tr);
				if( out_p != -1 )
					schedule_event(out_p);
			} else {
				backlogged_event_list.push(e);
			}
		}
		
		
	} else {
		// put this event into backlogged event list
		#ifdef _DEBUG_
		cout<<"Backlogged: ("<<sim_time<<", "<<t<<")"<<endl;
		#endif
		backlogged_event_list.push(Event(sim_time,t));
	}
	
}

void change_state(Transition t)
{
	// update the statistics counter
	update_stat();

	for( int p=0; p < n_place; p++) {
		marking[p] = marking[p]+ matrix[p*n_transition+t];
	}
	
	#ifdef _DEBUG_
	cout<<"After transition ";
	print_marking();
	#endif

}

// schedule an event at transition t
bool schedule_event(Place p)
{
	Transition t= getOutputTransition(p);
	double event_time = sim_time + getDwellTime(p);
	future_event_list.push(Event(event_time, t) );

	#ifdef _DEBUG_
	cout<<"Schedule event ("<< event_time<<", "<<t<<")"<<endl;
	#endif
	return true;
}

// get an output place of transition t 
Place getOutputPlace(Transition t)
{
	Place num_place = output_place[t].size();
	Place out_p;
	for( int p=0; p < num_place; p++)
	{
		out_p =output_place[t][p];
		if( !isResourcePlace(out_p) && !isIdlePlace(out_p) )
			return out_p;
	}
	return -1;
}

// test the specified transition t is firable
bool isFirable(Transition t)
{
	//cout<<"transition "<<t<<" size:"<<input_place[t].size()<<endl;
	Place num_place = input_place[t].size();
	for ( int i=0; i < num_place; i++ ) {
		//cout<<"test "<<input_place[t][i]<<endl;
		Place p = input_place[t][i];	// find an input place for transition t
		if( marking[p] <= 0 )	// check whether there are enough token in place
			return false;
	}
	return true;
}


bool isTerminated()
{
	if( equal(marking.begin(),marking.end(),
		init_marking.begin()) )
		return true;
	else
		return false;
}

bool isDeadlock()
{
	for( int i=0; i < n_deadlock_state; i++) {
		#ifdef _DEBUG_
		cout<<"Checking Deadlock..."<<endl;
		cout<<"Marking:\t";
		copy(marking.begin()+ n_idle_place, 
			marking.end()-n_resource_place, ostream_iterator<int>(cout,", ") );
		cout<<endl;
		cout<<"Deadlock state:\t";
		copy(deadlock_state.begin()+i*n_operation_place, 
			deadlock_state.begin()+(i+1)*n_operation_place, ostream_iterator<int>(cout,", ") );
		cout<<endl;
		#endif
		
		if( equal(marking.begin()+ n_idle_place, 
			marking.end()-n_resource_place,
			deadlock_state.begin()+i*n_operation_place,
			greater_equal<int>()) )
			return true;
	}
	return false;
}

bool isResourcePlace( Place p)
{
	if( p >= n_place - n_resource_place )
		return true;
	else
		return false;
}

bool isIdlePlace( Place p)
{
	if( p < n_idle_place )
		return true;
	else
		return false;
}

void read_matrix( const char* filename )
{
	ifstream fs(filename);

	read_data(fs, matrix);
	n_transition = matrix.size()/n_place;

	// pre-process the matrix
	#ifdef _DEBUG_
	cout<<"Pre-processing "<<n_place<<" x "
		<<n_transition<<" matrix...";
	#endif
	int value;
	input_place = new vector<Place>[n_transition];
	output_place = new vector<Place>[n_transition];
	output_transition = new vector<Transition>[n_place];
	output_tr_prob = new vector<double>[n_place];

	for(int t=0; t < n_transition; t++) {
		for(int p=0; p < n_place; p++) {
			value = matrix[n_transition*p+t];
			if( value <= -1 ) {
				input_place[t].push_back(p);
				output_transition[p].push_back(t);
			}
			else if(value >=1 && 
				p < n_place - n_resource_place )
				output_place[t].push_back(p);
		}

	}


	#ifdef _DEBUG_
	cout<<"done."<<endl;
	#endif

	#ifdef _DEBUG_
	for(int t=0; t<n_transition; t++){
		for(int p=0; p<input_place[t].size(); p++)
			cout<<input_place[t][p]<<" ";
		cout<<endl;
	}
	#endif
}

void read_initial_state( const char* filename )
{
	ifstream fs(filename);
	read_data(fs, init_marking);
	n_place = init_marking.size();

	if(n_place != n_idle_place +
		n_operation_place + n_resource_place ) {
		cerr<<"Inconsistent place specification!"<<endl;
		return;
	}

	marking.resize(n_place);
	std::copy(init_marking.begin(), init_marking.end(), marking.begin());

	in_place_time.resize(n_place);	// allocate space for statistics counter
}

void read_place_type( const char* filename )
{
	ifstream fs(filename);
	vector<Place> tmp;
	read_data(fs, tmp);
	n_idle_place = tmp[0];
	n_operation_place = tmp[1];
	n_resource_place = tmp[2];
	
}

void read_deadlock_state( const char* filename )
{
	ifstream fs(filename);
	read_data(fs, deadlock_state);
	n_deadlock_state = deadlock_state.size()/ n_operation_place;
}

void read_prob(const char* filename )
{
	vector<double> prob;
	ifstream fs(filename);
	if(fs.is_open()) {
	read_data(fs,prob);
	
	Place index =0;
	Place current_p = prob[index];

	for(Place p=0; p<n_place; p++) {
		if(p == current_p) {
			do {
				index++;
				output_tr_prob[p].push_back(prob[index]);
			} while(prob[index]<1.0);
			index++;
			if(index < prob.size() )
				current_p = prob[index];	// assert: prob[index-1]=1.0
		} else {
			output_tr_prob[p].push_back(1.0);
		}

		if( !isResourcePlace(p) && !isIdlePlace(p) && output_tr_prob[p].size() != output_transition[p].size() )
			cerr<<"Inconsistent matrix and probability input Files: at place "<<p<<endl;
	}

	} else {	// file not open
		for( Place p=0; p<n_place; p++)
			output_tr_prob[p].push_back(1.0);
	}
	
}

Transition getOutputTransition( Place p)
{
	Transition num_t = output_transition[p].size();
	double prob = lcgrand(100-stream);
	Transition output_t=output_transition[p][0];
	#ifdef _DEBUG_
	if(num_t > 1)
		cout<<"prob of choose branch one: "<<prob<<endl;
	#endif
	for ( Transition t=0; t < num_t; t++) {
		#ifdef _DEBUG_
		if(num_t > 1) cout<<"---"<<output_tr_prob[p][t]<<endl;
		#endif
		if(prob >= output_tr_prob[p][t])
			output_t = output_transition[p][t+1];
		else
			break;
	}
	return output_t;
}

void read_distribution( const char* filename)
{
	ifstream fs(filename);
	read_data(fs, mean_dwell_time);
	
	#ifdef _DEBUG_
	cout<<"Mean Dwell Time: ";
	for(int i=0;i<n_transition-n_resource_place; i++)
		cout<<mean_dwell_time[i]<<" ";
	cout<<endl;
	#endif
}

double getDwellTime( Transition t)
{
	return expon(mean_dwell_time[t], stream);
}

void update_stat()
{
	for( long i=0; i<n_place; i++)
	{
		in_place_time[i] += 
			marking[i] *
			(sim_time-prev_sim_time);
	}
	prev_sim_time = sim_time;
}

void print_stat(Place begin, Place end)
{
	for( long i=begin; i<end; i++) 
		cout<<in_place_time[i]/sim_time<<"\t";

}

void cleanup()
{
	delete input_place;
	delete output_place;
}

void print_marking()
{
	cout<<"current marking: ";
	for( int i=0; i<n_place; i++)
		cout<<marking[i];
	cout<<endl;
}

// initilize the petri-net model
void init()
{
	// initialize marking
	std::copy(init_marking.begin(), 
		init_marking.end(), marking.begin());

	// initialize the statitistics counter
	std::fill(in_place_time.begin(),
		in_place_time.end(), 0.0);
	sim_time = 0;
	prev_sim_time =0;

	// initialize the event list
	for( Place i=0; i<n_idle_place; i++) {
		if( marking[i] >=1 ) {
			for( long j=0; j<marking[i]; j++) {
				schedule_event(i);
			}
		}
	}


}

int main(int argc, char* argv[])
{
	
	long num_reps=100;
	stream=1;
	string dir=".";

	Place stat_begin = -1;
	Place stat_end = -1;

	// read stream number from command line argument
	if(argc < 3 ) {
		cout<<"\tPetri-net Simulator for Deadlock Detection"<<endl;
		cout<<"\tUsage: petri-net rep seed dir"<<endl;
		cout<<"\t\trep\tNumber of replications"<<endl;
		cout<<"\t\tseed\tRandom number seed for the lcgrand generator"<<endl;
		cout<<"\t\tdir\tDirectory where input files are stored"<<endl<<endl;
		
		return 0;
	}
	else {
		string str_seed(argv[2]), str_reps(argv[1]);
		std::istringstream ist1(str_seed);
		ist1>>stream;
		std::istringstream ist2(str_reps);
		ist2>>num_reps;
		if(argc >=4) {
			dir = argv[3];
		}

		if(argc>=6){
			string s_begin(argv[4]), s_end(argv[5]);
			std::istringstream ist4(s_begin);
			std::istringstream ist5(s_end);
			ist4>>stat_begin;
			ist5>>stat_end;
		}
	}
	
	read_place_type((dir+"/place-type.txt").c_str());
	read_initial_state((dir+"/marking.txt").c_str());
	read_matrix((dir+"/matrix.txt").c_str());
	read_distribution((dir+"/distribution.txt").c_str());
	read_deadlock_state((dir+"/deadlock-state.txt").c_str());
	read_prob((dir+"/probability.txt").c_str());

	if(stat_begin==-1) {
		stat_begin=0;//n_place-n_resource_place;
		stat_end = n_idle_place;//n_place;
	}
	
	#ifdef _DEBUG_
	cout<<" start random number generator with stream #"<<stream<<endl;
	#endif

	#ifndef _DEBUG_
	cout<<"status\tsim_time\t";
	for(int i=stat_begin; i<stat_end; i++)
		cout<<"in_"<<i<<"\t";
	cout<<endl;
	#endif
	long n_deadlock=0, n_finished=0;
	
	for( int i=0; i<num_reps; i++) {

		#ifdef _DEBUG_
		cout<<endl<<"============  Replication "<<i<<" ============"<<endl;
		#endif

		init();

		#ifdef _DEBUG_
		cout<<"sim_time="<<sim_time<<endl;
		print_marking();
		#endif

		while(1) {
			timing();
			if( isTerminated() ) {
				// properly terminated 
				cout<<"1\t"<<sim_time<<"\t";
				print_stat(stat_begin, stat_end);
				cout<<endl;
				n_finished++;
				break;
			}
			if( isDeadlock() ) {
				// deadlock occurred
				cout<<"0\t"<<sim_time<<"\t";
				print_stat(stat_begin, stat_end);
				cout<<endl;
				n_deadlock++;
				break;
			}
		}
	}
/*	
	cout<<"\n==================================="<<endl;
	cout<<"\nRun for "<<num_reps<<" replications"<<endl;
	cout<<n_finished<<" finished"<<endl;
	cout<<n_deadlock<<" deadlocked"<<endl;
*/	
	return 1;
	//cleanup();
}

