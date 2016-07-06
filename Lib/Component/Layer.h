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
  
    Layer() = default;
    inline Layer( const string &layer , double centerX  , double centerY ,
                                        double h        , double w );
    inline Layer( const string &layer ,  const Point &center ,  double h ,
                                                                double w );
    inline Layer( const string &layer ,  const Rectangle &rect );
  
    inline const string& layer() const;
    
    inline void setLayer( const string &layer );
    
    inline Layer operator+( const Point &p ) const;

  private:
  
    string mLayer;
};

ostream& operator<<( ostream &out , const Layer &layer );

inline Layer::Layer( const string &layer ,  double centerX  , double centerY ,
                                            double h        , double w )
  : Rectangle( centerX , centerY , h , w ) , mLayer( layer ) {}
inline Layer::Layer( const string &layer ,  const Point &center , double h ,
                                                                  double w )
  : Rectangle( center , h , w ) , mLayer( layer ) {}
inline Layer::Layer( const string &layer ,  const Rectangle &rect )
  : Rectangle( rect ) , mLayer( layer ) {}

inline const string& Layer::layer() const { return mLayer; }

inline void Layer::setLayer( const string &layer ) { mLayer = layer; }

inline Layer Layer::operator+( const Point &p ) const
{ return Layer( mLayer , center() + p , height() , width() ); }

#endif
