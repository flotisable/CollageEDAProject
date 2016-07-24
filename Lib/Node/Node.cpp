#include"Node.h"

// Node public member function
int Node::searchConnectNode( const string &name )
{
	for( unsigned int i = 0 ; i < mConnect.size() ; i++ )
		 if( mConnect[i]->name() == name ) return i;
	return -1;
}
// end Node public member function
