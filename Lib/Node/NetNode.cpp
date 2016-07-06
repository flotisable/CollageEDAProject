#include "NetNode.h"

#include <iomanip>

ostream& operator<<( ostream &out , NetNode &node )
{
  static const int TAB = 10;

  out << left;
  out << setw( TAB )  << node.name();
  out << node.type()  << "  ";
  
  for( Node *nodeConnect : node.connect() )
     out << setw( TAB ) << nodeConnect->name();

  for( auto &net : node.nets() )
     out << static_cast<Rectangle>( net ) << " ";

  return out;
}
