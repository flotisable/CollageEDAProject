#ifndef POINT_H
#define POINT_H

#include <ostream>

class Point // ÂI
{
  public:

    Point() = default;
    inline Point( double x , double y );

    inline double x() const;
    inline double y() const;

    inline void setX( double x );
    inline void setY( double y );
    inline void set ( double x , double y );

    inline Point  operator+ ( const Point &p ) const;
    inline Point  operator- ( const Point &p ) const;
    inline Point& operator+=( const Point &p );
    inline Point& operator-=( const Point &p );

    inline bool operator==( const Point &p ) const;

  private:

    double m_x; // x®y¼Ð
    double m_y; // y®y¼Ð
};

// Point non-member function
std::ostream& operator<<( std::ostream &out , const Point &point );
// end Point non-member function

// Point inline member funcion
inline Point::Point( double x , double y ) { set( x , y ); }

inline double Point::x() const { return m_x; }
inline double Point::y() const { return m_y; }

inline void Point::setX( double x )                   { m_x = x; }
inline void Point::setY( double y )                   { m_y = y; }
inline void Point::set ( double x , const double y )  { m_x = x;
                                                        m_y = y; }

inline Point Point::operator+( const Point &p ) const
{ return Point( m_x + p.x() , m_y + p.y() ); }
inline Point Point::operator-( const Point &p ) const
{ return Point( m_x - p.x() , m_y - p.y() ); }
inline Point& Point::operator+= ( const Point &p )
{ return *this = *this + p; }
inline Point& Point::operator-= ( const Point &p )
{ return *this = *this - p; }

inline bool Point::operator==( const Point &p ) const
{ return ( m_x == p.x() && m_y == p.y() ); }
// end Point inline member funcion

#endif
