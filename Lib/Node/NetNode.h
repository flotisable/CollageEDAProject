#ifndef NET_NODE_H
#define NET_NODE_H

#include <ostream>

#include "Node.h"

class NetNode : public Node
{
  public:

    inline NetNode();

    inline vector<Rectangle>& nets();

  private:

    vector<Rectangle> m_nets;
};

ostream& operator<<( ostream &out , NetNode &node );

// NetNode inline member function
inline NetNode::NetNode() : Node( NET ) {}

inline vector<Rectangle>& NetNode::nets() { return m_nets; }
// end NetNode inline member function

#endif
