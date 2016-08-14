#ifndef NET_NODE_H
#define NET_NODE_H

#include <ostream>
#include <string>
using namespace std;

#include "Node.h"
#include "../Component/Layer.h"
#include "../Component/ViaDevice.h"

class NetNode : public Node
{
  public:

    inline NetNode();
    inline NetNode( const string &name );

    inline vector<Layer>&     segments();
    inline vector<ViaDevice>& contacts();

  private:

    vector<Layer>     mSegments;
    vector<ViaDevice> mContacts;
};

// NetNode non-member function
ostream& operator<<( ostream &out , NetNode &node );
// end NetNode non-member function

// NetNode inline member function
inline NetNode::NetNode() : Node( NET ) {}
inline NetNode::NetNode( const string &name ) : Node( name , NET ) {}

inline vector<Layer>&     NetNode::segments() { return mSegments; }
inline vector<ViaDevice>& NetNode::contacts() { return mContacts; }
// end NetNode inline member function

#endif
