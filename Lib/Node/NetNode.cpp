#include "NetNode.h"

#include <iomanip>

// NetNode non-member function
ostream& operator<<( ostream &out , NetNode &node )
{
  const int TAB = 10;

  out << left;
  out << setw( TAB )  << node.name();
  out << node.type()  << "  ";
  
  for( Node *nodeConnect : node.connect() )
     out << setw( TAB ) << nodeConnect->name();

  for( auto &segment : node.segments() )
     out << static_cast<Rectangle>( segment ) << " ";

  return out;
}
// end NetNode non-member function
