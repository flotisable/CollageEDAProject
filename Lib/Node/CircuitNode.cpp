#include "CircuitNode.h"

#include <iomanip>

#include "../Model/CircuitModel.h"

// CircuitNode non-member function
ostream& operator<<( ostream &out , CircuitNode &node )
{
  const int TAB = 10;

  out << left;
  out << setw( TAB )    << node.name();
  out << node.type()    << "  ";
  out << node.center()  << "  ";

  out << left;
  out << setw( TAB )    << node.model()->name();

  for( Node* nodeConnect : node.connect() )
     out << setw( TAB ) << nodeConnect->name();

  return out;
}
// end CircuitNode non-member function
