
#ifndef _VECTOR_READER_H_
#define _VECTOR_READER_H_

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

template <typename T>
void read_data(std::istream& in_stream, std::vector<T>& vec, 
				string comment_sep="#",	// Seperator between data and comments
				string eof="END")		// End-of-file signal
{
	static const char whitespace[] = " \n\t\v\r\f";
	
	string nextline="";

	while( in_stream || nextline.length() > 0 )
	{
		string line;
		if( nextline.length() > 0 ) {
			line = nextline;	// we read ahead; use it now
			nextline = "";
		} else {
			std::getline(in_stream, line);
		}
		
		// ignore comments
		line = line.substr( 0, line.find(comment_sep) );
		//debug
		//cout<<"line without comment"<<line<<endl;

		// check for end-of-file signal
		if( eof != "" && line.find(eof) != string::npos ) return;
		
		// trim the leading and trailing spaces
		line.erase( 0, line.find_first_not_of(whitespace) );
		line.erase( line.find_last_not_of(whitespace) + 1U );

		// if line is empty, return
		if( line == "" ) continue;
		
		//debug
		//cout<<"The line is: "<<line<<endl;

		typedef std::istream_iterator<T>  is_iter;
		std::istringstream ist(line);
		
		// read all elements in this line into a vector
		std::copy( is_iter(ist), is_iter(),
					std::back_inserter(vec) );
		
		// read next line
		std::getline(in_stream, nextline);
	}		
}

#endif
