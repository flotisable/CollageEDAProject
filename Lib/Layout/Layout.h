#ifndef LAYOUT_H
#define LAYOUT_H

#include <string>
#include <fstream>
using namespace std;

#include "../Graphic/Point.h"
#include "../Graphic/Rectangle.h"
#include "../Component/Layer.h"

class Mos;
class ViaDevice;
class Circuit;

class Layout
{
  public:

    inline  Layout  ( const char *fileName = "Layout.txt" );
    virtual ~Layout () = default;

    inline bool setFile( const char *fileName );

    inline void setCenter( const Point &p       );
    inline void setCenter( double x , double y  );

    inline bool   drawRect( const string &layer , const Rectangle &rect );
    inline bool   drawRect( const string &layer , const Point &lb ,
                                                  const Point &rt );
    virtual bool  drawRect( const string &layer ,
                            double lbX , double lbY ,
                            double rtX , double rtY ) = 0;

    inline bool drawLayer( const Layer &layer );

    bool drawMos      ( Mos       *mos        );
    bool drawViaDevice( ViaDevice *viaDevice  );
    bool drawCircuit  ( Circuit   *circuit    );

  protected:

    fstream file;
    Point   center;
};

inline Layout::Layout( const char *fileName )
{ if( fileName ) file.open( fileName , ios::out ); }

inline bool Layout::setFile( const char *fileName )
{
  if( file.is_open() ) file.close();
  if( fileName ) file.open( fileName , ios::out );
  return file.is_open();
}

inline void Layout::setCenter( const Point &p      )
{ center = p;               }
inline void Layout::setCenter( double x , double y )
{ center = Point( x , y );  }

inline bool Layout::drawRect( const string &layer , const Rectangle &rect )
{
  return drawRect( layer ,  rect.left () , rect.bottom() ,
                            rect.right() , rect.top   () );
}

inline bool Layout::drawRect( const string &layer , const Point &lb ,
                                                    const Point &rt )
{ return drawRect( layer , lb.x() , lb.y() , rt.x() , rt.y() ); }

inline bool Layout::drawLayer( const Layer &layer )
{ return drawRect( layer.layer() ,  layer.left  () , layer.bottom () ,
                                    layer.right () , layer.top    () ); }

#endif
