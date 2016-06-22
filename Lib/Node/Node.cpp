#include"Node.h"

int Node::searchConnectNode( const string &name )
{
	for( register int i = 0 ; i < static_cast<int>( m_connect.size() ) ; i++ )
		 if( m_connect[i]->name() == name ) return i;
	return -1;
}
