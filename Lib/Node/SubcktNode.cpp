#include "SubcktNode.h"

#include <iomanip>

#include "../Model/SubcktModel.h"

ostream& operator<<( ostream &out , SubcktNode &node )
{
  static const int TAB = 10;
  
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
