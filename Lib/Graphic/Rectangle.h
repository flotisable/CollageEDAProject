#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <ostream>

#include "Point.h"

class Rectangle // ¯x§Î
{
  public:

    Rectangle() = default;
    Rectangle( double centerX , double centerY  , double h , double w );
    inline Rectangle( const Point &center , double h , double w );

    inline const Point& center() const;
    inline double       width () const;
    inline double       height() const;

    inline void setCenter ( double x , double y );
    inline void setCenter ( const Point   &p    );
    inline void setCenterX( double        x     );
    inline void setCenterY( double        y     );
    inline void setWidth  ( double        w     );
    inline void setHeight ( double        h     );

    inline double area  () const;
    inline double top   () const;
    inline double bottom() const;
    inline double left  () const;
    inline double right () const;
    
    inline Rectangle operator+( const Point &p ) const;
		
  private:

		Point   m_center;
    double  m_height;
    double  m_width;
};

// Rectangle non-member function
std::ostream& operator<<( std::ostream &out , const Rectangle &rect );
// end Rectangle non-member function

// Rectangle inline member function
inline Rectangle::Rectangle( const Point &center , double h , double w )
: Rectangle( center.x() , center.y() , h , w ) {}

inline const Point& Rectangle::center() const { return m_center; }
inline double       Rectangle::width () const { return m_width;  }
inline double       Rectangle::height() const { return m_height; }

inline void Rectangle::setCenter  ( double x , double y )
{ m_center  = Point( x , y ); }
inline void Rectangle::setCenter  ( const Point &p      )
{ m_center  = p;              }
inline void Rectangle::setCenterX ( double      x       )
{ m_center.setX( x );         }
inline void Rectangle::setCenterY ( double      y       )
{ m_center.setY( y );         }
inline void Rectangle::setWidth   ( double      w       )
{ m_width   = w;              }
inline void Rectangle::setHeight  ( double      h       )
{ m_height  = h;              }

inline double Rectangle::area  () const
{ return m_width       * m_height;     }
inline double Rectangle::top   () const
{ return m_center.y()  + m_height / 2; }
inline double Rectangle::bottom() const
{ return m_center.y()  - m_height / 2; }
inline double Rectangle::left  () const
{ return m_center.x()  - m_width  / 2; }
inline double Rectangle::right () const
{ return m_center.x()  + m_width  / 2; }

inline Rectangle Rectangle::operator+( const Point &p ) const
{ return Rectangle( m_center + p , m_height , m_width ); }
// end Rectangle inline member function

#endif
