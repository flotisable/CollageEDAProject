#include"Node.h"

int Node::searchConnectNode( const string &name )
{
	for( register unsigned int i = 0 ; i < mConnect.size() ; i++ )
		 if( mConnect[i]->name() == name ) return i;
	return -1;
}
