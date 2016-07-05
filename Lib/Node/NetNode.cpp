#include "NetNode.h"

#include <iomanip>

ostream& operator<<( ostream &out , NetNode &node )
{
  static const int TAB = 10;

  out << left;
  out << setw( TAB )  << node.name();
  out << node.type()  << "  ";
  
  for( unsigned int i = 0 ; i < node.connect().size() ; i++ )
     out << setw( TAB ) << node.connect()[i]->name();

  for( unsigned int i = 0 ; i < node.nets().size() ; i++ )
     out << node.nets()[i] << " ";

  return out;
}
