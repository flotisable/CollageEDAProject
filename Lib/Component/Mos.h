#ifndef MOS_H
#define MOS_H

#include <ostream>
#include <vector>
using namespace std;

#include "../Graphic/Rectangle.h"

class TechFile;

class Mos
{
  public:

    enum Device
    {
      METAL,
      CONTACT
    };

    enum Type
    {
      UNKNOWN = -1,
      NMOS,
      PMOS
    };

    inline Mos();
    Mos(  int type , double w , double l , unsigned int m ,
          TechFile *techFile = nullptr );

    inline int          type() const;
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
    
    inline const Rectangle&         diffusion () const;
    inline const vector<Rectangle>& source    () const;
    inline const Rectangle&         gate      () const;
    inline const vector<Rectangle>& drain     () const;
    inline const Rectangle&         implant   () const;

    inline bool operator==( const Mos &mos );

  private:

    int           m_type;
    double        m_w;
    double        m_l;
    unsigned int  m_m;
    
    Rectangle         diff;
    vector<Rectangle> s;
    Rectangle         g;
    vector<Rectangle> d;
    Rectangle         imp;
    
    TechFile *tech;
    
    void writeLayer(  ostream &output , const char *name ,
                      const Rectangle &layer );
};

// Mos non-memeber function
ostream& operator<<( ostream &out , Mos &mos );
// end Mos non-memeber function

// Mos inline member function
inline Mos::Mos() : m_type( UNKNOWN ) , tech( nullptr ) {}

inline int          Mos::type() const { return m_type; }
inline double       Mos::w   () const { return m_w;    }
inline double       Mos::l   () const { return m_l;    }
inline unsigned int Mos::m   () const { return m_m;    }

inline void Mos::setType( Type          t ) { m_type  = t; }
inline void Mos::setW   ( double        w ) { m_w     = w; }
inline void Mos::setL   ( double        l ) { m_l     = l; }
inline void Mos::setM   ( unsigned int  m ) { m_m     = m; }

inline void Mos::setTechFile( TechFile *techFile ) { tech = techFile; }

inline const Rectangle&         Mos::diffusion() const { return diff; }
inline const vector<Rectangle>& Mos::source   () const { return s   ; }
inline const Rectangle&         Mos::gate     () const { return g   ; }
inline const vector<Rectangle>& Mos::drain    () const { return d   ; }
inline const Rectangle&         Mos::implant  () const { return imp ; }

inline bool Mos::operator==( const Mos &mos )
{
  return (  m_type  == mos.type() && m_w == mos.w() &&
            m_l     == mos.l()    && m_m == mos.m() );
}
// end Mos inline member function

#endif
