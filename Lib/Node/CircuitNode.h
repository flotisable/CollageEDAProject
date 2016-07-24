#ifndef CIRCUIT_NODE_H
#define CIRCUIT_NODE_H

#include <ostream>
#include <string>
using namespace std;

#include "Node.h"

class CircuitModel;

class CircuitNode : public Node
{
  public:

    inline CircuitNode( CircuitModel *model = nullptr );
    inline CircuitNode( const string &name , CircuitModel *model = nullptr );

    inline CircuitModel* model() const;

    inline void setModel( CircuitModel *model );

  private:

    CircuitModel *circuit;
};

// CircuitNode non-member function
ostream& operator<<( ostream &out , CircuitNode &node );
// end CircuitNode non-member function

// CircuitNode inline member function
inline CircuitNode::CircuitNode( CircuitModel *model )
  : Node( CIRCUIT ) , circuit( model ) {}
inline CircuitNode::CircuitNode( const string &name , CircuitModel *model )
  : Node( name , CIRCUIT ) , circuit( model ) {}

inline CircuitModel* CircuitNode::model() const { return circuit; }

inline void CircuitNode::setModel( CircuitModel *model ) { circuit = model; }
// end CircuitNode inline member function

#endif
