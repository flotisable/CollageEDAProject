#ifndef NET_NODE_H
#define NET_NODE_H

#include <ostream>

#include "Node.h"
#include "../Component/Layer.h"

class NetNode : public Node
{
  public:

    inline NetNode();

    inline vector<Layer>& nets();

  private:

    vector<Layer> mNets;
};

ostream& operator<<( ostream &out , NetNode &node );

// NetNode inline member function
inline NetNode::NetNode() : Node( NET ) {}

inline vector<Layer>& NetNode::nets() { return mNets; }
// end NetNode inline member function

#endif
