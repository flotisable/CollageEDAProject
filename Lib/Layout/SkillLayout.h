#ifndef SKILL_LAYOUT_H
#define SKILL_LAYOUT_H

#include <string>
#include <fstream>
using namespace std;

#include "../Graphic/Point.h"
#include "../Graphic/Rectangle.h"

class Mos;
class ViaDevice;
class ICModel;

class SkillLayout
{
  public:
  
    inline SkillLayout( const char *fileName = "skillLayout.il" );

    inline bool setFile( const char *fileName );
    
    inline void setCenter( const Point &p       );
    inline void setCenter( double x , double y  );

    inline bool drawRect( const string &layer , const Rectangle &rect   );
    inline bool drawRect( const string &layer , const Point &lb ,
                                                const Point &rt );
    bool drawRect( const string &layer ,  double lbX , double lbY ,
                                          double rtX , double rtY );

    inline bool drawInst( const string &lib , const string &cell ,
                          const string &view ,
                          const Point &pos , double rotate );
    bool drawInst(  const string &lib , const string &cell ,
                    const string &view ,
                    double x , double y , double rotate );

    bool drawMos      ( Mos       *mos        );
    bool drawViaDevice( ViaDevice *viaDevice  );
    bool drawSubckt   ( ICModel   *subckt     );
  
  private:

    static const string GET_REPRESENT;
    static const string GET_CELLVIEW;
    static const string DRAW_RECTANGLE;
    static const string DRAW_INSTANCE;
  
    fstream file;
    Point   center;
};

inline SkillLayout::SkillLayout( const char *fileName )
{ if( fileName ) file.open( fileName , ios::out ); }

inline bool SkillLayout::setFile( const char *fileName )
{
  if( file.is_open() ) file.close();
  if( fileName ) file.open( fileName , ios::out );
  return file.is_open();
}

inline void SkillLayout::setCenter( const Point &p      )
{ center = p;               }
inline void SkillLayout::setCenter( double x , double y )
{ center = Point( x , y );  }

inline bool SkillLayout::drawRect(  const string &layer ,
                                    const Rectangle &rect )
{
  return drawRect( layer ,  rect.left () , rect.bottom() ,
                            rect.right() , rect.top   () );
}

inline bool SkillLayout::drawRect( const string &layer ,  const Point &lb ,
                                                          const Point &rt )
{ return drawRect( layer , lb.x() , lb.y() , rt.x() , rt.y() ); }

inline bool SkillLayout::drawInst(  const string  &lib   ,
                                    const string  &cell  ,
                                    const string  &view  ,
                                    const Point   &pos , double rotate )
{ return drawInst( lib , cell , view , pos.x() , pos.y() , rotate ); }

#endif
