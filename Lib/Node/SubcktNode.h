#ifndef SUBCKT_NODE_H
#define SUBCKT_NODE_H

#include <ostream>

#include "Node.h"

class SubcktModel;

class SubcktNode : public Node
{
  public:

    inline SubcktNode( SubcktModel *model = nullptr );

    inline SubcktModel* model() const;

    inline void setModel( SubcktModel *model );

  private:

    SubcktModel *subckt;
};

ostream& operator<<( ostream &out , SubcktNode &node );

// SubcktNode inline member function
inline SubcktNode::SubcktNode( SubcktModel *model )
: Node( SUBCKT ) , subckt( model ) {}

inline SubcktModel* SubcktNode::model() const { return subckt; }

inline void SubcktNode::setModel( SubcktModel *model ) { subckt = model; }
// end SubcktNode inline member function

#endif
