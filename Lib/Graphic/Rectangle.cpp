#include "Rectangle.h"

#include <iomanip>

// Rectangle member function
Rectangle::Rectangle( double centerX  , double centerY ,
                      double h        , double w          )
{
  mCenter  = Point( centerX , centerY );
  mHeight  = h;
  mWidth   = w;
}
// end Rectangle member function

// Rectangle non-member function
std::ostream& operator<<( std::ostream &out , const Rectangle &rect )
{
  const int TAB = out.precision();

  return out  << std::right       << rect.center() << " "
              << std::setw( TAB ) << rect.height() << " "
              << std::setw( TAB ) << rect.width ();
}
// Rectangle non-member function
