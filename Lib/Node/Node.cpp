#include"Node.h"

int Node::searchConnectNode( const string &name )
{
	for( register unsigned int i = 0 ; i < m_connect.size() ; i++ )
		 if( m_connect[i]->name() == name ) return i;
	return -1;
}
