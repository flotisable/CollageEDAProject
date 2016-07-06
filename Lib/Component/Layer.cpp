#include "Layer.h"

#include <iomanip>

ostream& operator<<( ostream &out , const Layer &layer )
{
  static const int TAB = 10;
  
  out << left;
  out << setw( TAB ) << layer.layer();
  out << static_cast<Rectangle>( layer );
  
  return out;
}
