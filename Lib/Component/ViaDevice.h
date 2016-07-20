#ifndef VIA_DEVICE_H
#define VIA_DEVICE_H

#include <string>
#include <vector>
using namespace std;

#include "../Graphic/Point.h"
#include "Layer.h"

class TechFile;

class ViaDevice
{
  public:
  
    enum Type
    {
      UNKNOWN,
      N,
      P
    };

    ViaDevice(  const   string &viaLayer = "CONT" ,
                double  centerX = 0 , double  centerY = 0 ,
                int     row     = 1 , int     col     = 1 ,
                TechFile *techFile = nullptr );

    inline ViaDevice( const string &viaLayer , const Point &center ,
                      int row , int col , TechFile *techFile = nullptr );
    
  
    inline const string&  viaLayer() const;
    inline int            type    () const;
    inline const Point&   center  () const;
    inline int            row     () const;
    inline int            column  () const;
    
    inline void setViaLayer ( const string  &viaLayer );
    inline void setType     ( Type          type      );
    inline void setCenter   ( const Point   &center   );
    inline void setCenter   ( double x , double y     );
    void        setRow      ( unsigned int  row       );
    void        setColumn   ( unsigned int  col       );
  
    inline void setTechFile( TechFile *techFile );
    
    void generate();
    
    inline const vector<vector<Layer>>& contact   () const;
    inline const Layer&                 metal     () const;
    inline const Layer&                 diffusion () const;
    inline const Layer&                 implant   () const;
  
  private:
  
    TechFile  *tech;
    
    string        via;
    int           mType;
    Point         mCenter;
    unsigned int  mRow;
    unsigned int  mCol;
    
    vector<vector<Layer>> contacts;
    Layer                 m;
    Layer                 diff;
    Layer                 imp;
};

inline ViaDevice::ViaDevice(  const string &viaLayer , const Point &center ,
                              int row , int col , TechFile *techFile )
: ViaDevice( viaLayer , center.x() , center.y() , row , col , techFile ) {}

inline const string&  ViaDevice::viaLayer() const { return via;     }
inline int            ViaDevice::type    () const { return mType;   }
inline const Point&   ViaDevice::center  () const { return mCenter; }
inline int            ViaDevice::row     () const { return mRow;    }
inline int            ViaDevice::column  () const { return mCol;    }

inline void ViaDevice::setViaLayer( const string  &viaLayer )
{ via       = viaLayer;       }
inline void ViaDevice::setType    ( Type          type      )
{ mType    = type;            }
inline void ViaDevice::setCenter  ( const Point   &center   )
{ mCenter  = center;          }
inline void ViaDevice::setCenter  ( double x , double y     )
{ mCenter  = Point( x , y );  }

inline void ViaDevice::setTechFile( TechFile *techFile )
{ tech = techFile; }

inline const vector<vector<Layer>>& ViaDevice::contact  () const
{ return contacts; }
inline const Layer&                 ViaDevice::metal    () const
{ return m;        }
inline const Layer&                 ViaDevice::diffusion() const
{ return diff;     }
inline const Layer&                 ViaDevice::implant  () const
{ return imp;      }

#endif
