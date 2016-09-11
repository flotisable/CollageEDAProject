#ifndef LAYER_H
#define LAYER_H

#include <string>
#include <ostream>
using namespace std;

#include "../Graphic/Rectangle.h"
#include "../Graphic/Point.h"

class Layer : public Rectangle
{
  public:

    enum Type
    {
      DIFFUSION,
      POLY1,
      METAL1,
      METAL2,
      METAL3,
      METAL4,
      METAL5,
      METAL6,
      CONTACT,
      VIA12,
      VIA23,
      VIA34,
      VIA45,
      VIA56,
      NIMPLANT,
      PIMPLANT,
      NWELL,
      PWELL,
      TYPE_NUM,
      UNKNOWN
    };
    
    static inline void setTypesName( Type type , const string &name );

    static bool read  ( const char *fileName = "layerTypes.txt" );
    static bool write ( const char *fileName = "layerTypes.txt" );
    
    static Type           map( const string &type );
    static inline string  map( Type         type  );

    Layer() = default;
    inline Layer( Type type , double centerX  , double centerY ,
                              double h        , double w );
    inline Layer( Type type , const Point &center , double h , double w );
    inline Layer( Type type , const Rectangle &rect );
    inline Layer( Type type , double headPin , double tailPin , double track );

    inline Type   type    () const;
    inline double headPin () const;
    inline double tailPin () const;
    inline double track   () const;

    inline void setType   ( Type  type    );
    inline void setPin    ( int   headPin , int tailPin );
    inline void setHeadPin( int   headPin );
    inline void setTailPin( int   tailPin );
    inline void setTrack  ( int   track   );

    inline Layer operator+( const Point &p ) const;

  private:

    static string TYPES[TYPE_NUM];
  
    Type mType;
};

ostream& operator<<( ostream &out , const Layer &layer );

inline Layer::Layer( Type type ,  double centerX  , double centerY ,
                                  double h        , double w )
  : Rectangle( centerX , centerY , h , w ) , mType( type ) {}
inline Layer::Layer( Type type , const Point &center , double h , double w )
  : Rectangle( center , h , w ) , mType( type ) {}
inline Layer::Layer( Type type , const Rectangle &rect )
  : Rectangle( rect ) , mType( type ) {}
inline Layer::Layer( Type type ,  double headPin ,
                                  double tailPin , double track )
  : mType( type )
{ setHeadPin( headPin ); setTailPin( tailPin ); setTrack( track ); }

inline void Layer::setTypesName( Type type , const string &name )
{
  if( type == UNKNOWN ) return;
  else                  TYPES[type] = name;
}

inline string Layer::map( Type type )
{ return ( type == UNKNOWN ) ? "UNKNOWN" : TYPES[type] ; }

inline Layer::Type  Layer::type   () const { return mType;        }
inline double       Layer::headPin() const { return center().x(); }
inline double       Layer::tailPin() const { return center().y(); }
inline double       Layer::track  () const { return width();      }

inline void Layer::setType    ( Type type     ) { mType = type;           }
inline void Layer::setPin     ( int  headPin , int tailPin )
{ setCenter( headPin , tailPin ); }
inline void Layer::setHeadPin ( int  headPin  ) { setCenterX( headPin );  }
inline void Layer::setTailPin ( int  tailPin  ) { setCenterY( tailPin );  }
inline void Layer::setTrack   ( int  track    ) { setWidth( track );      }

inline Layer Layer::operator+( const Point &p ) const
{ return Layer( mType , center() + p , height() , width() ); }

#endif
