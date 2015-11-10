
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

void trim( string& s )
{
	static const char whitespace[] = " \n\t\v\r\f";
	s.erase( 0, s.find_first_not_of(whitespace) );
	s.erase( s.find_last_not_of(whitespace) + 1U );
}

template <typename T>
void read_data(std::istream& in_stream, std::vector<T>& vec, 
				string delim="\t",	// delimiter of data
				string comment_sep="#",	// Seperator between data and comments
				string eof="END")		// End-of-file signal
{
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
		
		// check for end-of-file signal
		if( eof != "" && line.find(eof != string::npos ) return;
		
		trim(line);
		if( line == "" ) return;
		
		typedef std::istream_iterator<T> is_iter;
		std::istringstream ist(line);
		
		std::copy( is_iter(ist), is_iter(),
					std::back_inserter(vec) );
		
		std::getline(in_stream, nextline);
	}		
}
	
