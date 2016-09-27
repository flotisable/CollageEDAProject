#ifndef VIA_DEVICE_H
#define VIA_DEVICE_H

#include <vector>
using namespace std;

#include "../Graphic/Point.h"
#include "Layer.h"

class ViaDevice
{
  typedef Point Constrain;

  public:

    ViaDevice(  Layer::Type viaLayer = Layer::CONTACT ,
                double  centerX = 0 , double  centerY = 0 ,
                int     row     = 1 , int     col     = 1  );

    inline ViaDevice( Layer::Type viaLayer , const Point &center ,
                      int row , int col );

    inline Layer::Type  viaLayer  () const;
    inline const Point& center    () const;
    inline unsigned int row       () const;
    inline unsigned int column    () const;
    inline bool         impAllowed() const;

    void        setViaLayer   ( Layer::Type   viaLayer  );
    inline void setCenter     ( const Point   &center   );
    inline void setCenter     ( double x , double y     );
    void        setRow        ( unsigned int  row       );
    void        setColumn     ( unsigned int  col       );
    inline void setImpAllowed ( bool allowed            );

    inline void setLowerLayer ( Layer::Type layer       );
    inline void setImpLayer   ( Layer::Type layer       );
    inline void setConWidth   ( double      distance    );
    inline void setConSpace   ( double      distance    );
    inline void setConInUpper ( double      distanceX ,
                                double      distanceY   );
    inline void setConInUpper ( const Point &constrain  );
    inline void setConInLower ( double      distanceX ,
                                double      distanceY   );
    inline void setConInLower ( const Point &constrain  );
    inline void setLowerInImp ( double      distanceX ,
                                double      distanceY   );
    inline void setLowerInImp ( const Point &constrain  );

    void generate();

    inline const vector<vector<Layer>>& contact   () const;
    inline const Layer&                 upperLayer() const;
    inline const Layer&                 lowerLayer() const;
    inline const Layer&                 implant   () const;

  private:

    Layer::Type   via;
    Point         mCenter;
    unsigned int  mRow;
    unsigned int  mCol;
    bool          mImpAllowed;

    vector<vector<Layer>> contacts;
    Layer                 upper;
    Layer                 lower;
    Layer                 imp;

    double    conWidth;
    double    conSpace;
    Constrain conInUpper;
    Constrain conInLower;
    Constrain lowerInImp;
};

inline ViaDevice::ViaDevice(  Layer::Type viaLayer , const Point &center ,
                              int row , int col )
: ViaDevice( viaLayer , center.x() , center.y() , row , col ) {}

inline Layer::Type  ViaDevice::viaLayer   () const { return via;          }
inline const Point& ViaDevice::center     () const { return mCenter;      }
inline unsigned int ViaDevice::row        () const { return mRow;         }
inline unsigned int ViaDevice::column     () const { return mCol;         }
inline bool         ViaDevice::impAllowed () const { return mImpAllowed;  }

inline void ViaDevice::setCenter    ( const Point &center   )
{ mCenter     = center;         }
inline void ViaDevice::setCenter    ( double x , double y   )
{ mCenter     = Point( x , y ); }
inline void ViaDevice::setImpAllowed( bool allowed )
{ mImpAllowed = allowed;        }

inline void ViaDevice::setLowerLayer( Layer::Type layer       )
{ if( via == Layer::CONTACT ) lower.setType( layer ); }
inline void ViaDevice::setImpLayer  ( Layer::Type layer       )
{ if( lower.type() == Layer::DIFFUSION ) imp.setType( layer ); }
inline void ViaDevice::setConWidth  ( double      distance    )
{ conWidth    = distance; }
inline void ViaDevice::setConSpace  ( double      distance    )
{ conSpace    = distance; }
inline void ViaDevice::setConInUpper( double      distanceX ,
                                      double      distanceY   )
{ conInUpper  = Constrain( distanceX , distanceY ); }
inline void ViaDevice::setConInUpper( const Point &constrain  )
{ conInUpper  = constrain; }
inline void ViaDevice::setConInLower( double      distanceX ,
                                      double      distanceY   )
{ conInLower  = Constrain( distanceX , distanceY ); }
inline void ViaDevice::setConInLower( const Point &constrain  )
{ conInLower  = constrain; }
inline void ViaDevice::setLowerInImp( double      distanceX ,
                                      double      distanceY   )
{ lowerInImp  = Constrain( distanceX , distanceY ); }
inline void ViaDevice::setLowerInImp( const Point &constrain  )
{ lowerInImp  = constrain; }

inline const vector<vector<Layer>>& ViaDevice::contact    () const
{ return contacts; }
inline const Layer&                 ViaDevice::upperLayer () const
{ return upper;    }
inline const Layer&                 ViaDevice::lowerLayer () const
{ return lower;    }
inline const Layer&                 ViaDevice::implant    () const
{ return imp;      }

#endif
