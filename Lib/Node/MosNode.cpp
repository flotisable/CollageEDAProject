#include "MosNode.h"

#include <iomanip>

#include "../Model/MosModel.h"

// MosNode non-member function
ostream& operator<<( ostream &out , MosNode &node )
{
  const int     TAB     = 10;
  const double  &F_TAB  = out.precision();

  out << left;
  out << setw( TAB )    << node.name();
  out << node.type()    << "  ";
  out << node.center()  << "  ";

  out << right;
  out << node.model()->Mos::type()  << "  ";
  out << setw( F_TAB )              << node.model()->w();
  out << setw( F_TAB )              << node.model()->l();
  out << "  ";
  out << node.model()->m()          << "  ";
  
  out << left;
  
  for( Node* nodeConnect : node.connect() )
     out << setw( TAB ) << nodeConnect->name();

  return out;
}
// end MosNode non-member function
