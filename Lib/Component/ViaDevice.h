#ifndef VIA_DEVICE_H
#define VIA_DEVICE_H

#include <string>
#include <vector>
using namespace std;

#include "../Graphic/Point.h"
#include "../Graphic/Rectangle.h"

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
    
    inline const vector<vector<Rectangle>>& contact   () const;
    inline const Rectangle&                 metal     () const;
    inline const Rectangle&                 diffusion () const;
    inline const Rectangle&                 implant   () const;
  
  private:
  
    TechFile  *tech;
    
    string  via;
    int     m_type;
    Point   m_center;
    int     m_row;
    int     m_col;
    
    vector<vector<Rectangle>> contacts;
    Rectangle                 m;
    Rectangle                 diff;
    Rectangle                 imp;
};

inline ViaDevice::ViaDevice(  const string &viaLayer , const Point &center ,
                              int row , int col , TechFile *techFile )
: ViaDevice( viaLayer , center.x() , center.y() , row , col , techFile ) {}

inline const string&  ViaDevice::viaLayer() const { return via;       }
inline int            ViaDevice::type    () const { return m_type;    }
inline const Point&   ViaDevice::center  () const { return m_center;  }
inline int            ViaDevice::row     () const { return m_row;     }
inline int            ViaDevice::column  () const { return m_col;     }

inline void ViaDevice::setViaLayer( const string  &viaLayer )
{ via       = viaLayer;       }
inline void ViaDevice::setType    ( Type          type      )
{ m_type    = type;           }
inline void ViaDevice::setCenter  ( const Point   &center   )
{ m_center  = center;         }
inline void ViaDevice::setCenter  ( double x , double y     )
{ m_center  = Point( x , y ); }

inline void ViaDevice::setTechFile( TechFile *techFile )
{ tech = techFile; }

inline const vector<vector<Rectangle>>& ViaDevice::contact  () const
{ return contacts; }
inline const Rectangle&                 ViaDevice::metal    () const
{ return m;        }
inline const Rectangle&                 ViaDevice::diffusion() const
{ return diff;     }
inline const Rectangle&                 ViaDevice::implant  () const
{ return imp;      }

#endif
