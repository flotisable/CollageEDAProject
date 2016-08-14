#ifndef MOS_H
#define MOS_H

#include <ostream>
#include <vector>
using namespace std;

#include "Layer.h"
#include "ViaDevice.h"

class TechFile;

class Mos
{
  public:

    enum Type
    {
      UNKNOWN = -1,
      NMOS,
      PMOS
    };

    Mos(  Type type = UNKNOWN , TechFile *techFile = nullptr );
    inline Mos( Type type , double w , double l , unsigned int m ,
                TechFile *techFile = nullptr );

    inline Type         type() const;
    inline double       w   () const;
    inline double       l   () const;
    inline unsigned int m   () const;
    
    inline void setType ( Type          t );
    inline void setW    ( double        w );
    inline void setL    ( double        l );
    inline void setM    ( unsigned int  m );

    inline void setTechFile( TechFile *techFile );
    
    void generate ();
    bool write    ( const char *fileName = "mosInfo.txt"  );
    bool read     ( const char *fileName = "mosInput.txt" );

    inline const Layer&     diffusion () const;
    inline const ViaDevice& source    () const;
    inline const Layer&     gate      () const;
    inline const ViaDevice& drain     () const;
    inline const Layer&     implant   () const;

    inline bool operator==( const Mos &mos );

  private:

    Type          mType;
    double        mW;
    double        mL;
    unsigned int  mM;
    
    Layer     diff;
    ViaDevice s;
    Layer     g;
    ViaDevice d;
    Layer     imp;
    
    TechFile *tech;
    
    void writeLayer(  ostream &output , const char *name ,
                      const Layer &layer );
};

// Mos non-memeber function
ostream& operator<<( ostream &out , Mos &mos );
// end Mos non-memeber function

// Mos inline member function
inline Mos::Mos(  Type type , double w , double l , unsigned int m ,
                  TechFile *techFile ) : Mos( type , techFile )
{
  mW = w;
  mL = l;
  mM = m;
}

inline Mos::Type    Mos::type() const { return mType; }
inline double       Mos::w   () const { return mW;    }
inline double       Mos::l   () const { return mL;    }
inline unsigned int Mos::m   () const { return mM;    }

inline void Mos::setType( Type          t ) { mType  = t; }
inline void Mos::setW   ( double        w ) { mW     = w; }
inline void Mos::setL   ( double        l ) { mL     = l; }
inline void Mos::setM   ( unsigned int  m ) { mM     = m; }

inline void Mos::setTechFile( TechFile *techFile ) { tech = techFile; }

inline const Layer&     Mos::diffusion() const { return diff; }
inline const ViaDevice& Mos::source   () const { return s   ; }
inline const Layer&     Mos::gate     () const { return g   ; }
inline const ViaDevice& Mos::drain    () const { return d   ; }
inline const Layer&     Mos::implant  () const { return imp ; }

inline bool Mos::operator==( const Mos &mos )
{
  return (  mType == mos.type () && mW == mos.w() &&
            mL    == mos.l    () && mM == mos.m() );
}
// end Mos inline member function

#endif
