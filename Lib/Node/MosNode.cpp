#include "MosNode.h"

#include <iomanip>

#include "../Model/MosModel.h"
#include "../Component/Mos.h"

ostream& operator<<( ostream &out , MosNode &node )
{
  static const int  TAB   = 10;
  const double      F_TAB = out.precision();

  out << left;
  out << setw( TAB )    << node.name();
  out << node.type()    << "  ";
  out << node.center()  << "  ";
  
  Mos *mos = node.model()->model();
  out << right;
  out << mos->type()    << "  ";
  out << setw( F_TAB )  << mos->w();
  out << setw( F_TAB )  << mos->l();
  out << "  ";
  out << mos->m()       << "  ";
  
  out << left;
  
  for( unsigned int i = 0 ; i < node.connect().size() ; i++ )
     out << setw( TAB ) << node.connect()[i]->name();

  return out;
}
