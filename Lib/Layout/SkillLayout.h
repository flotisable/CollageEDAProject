#ifndef SKILL_LAYOUT_H
#define SKILL_LAYOUT_H

#include "Layout.h"

#include <string>
using namespace std;

#include "../Graphic/Point.h"

class SkillLayout : public Layout
{
  public:
  
    inline  SkillLayout ( const char *fileName = "skillLayout.il" );

    inline bool drawRect( const string &layer , const Rectangle &rect );
    inline bool drawRect( const string &layer , const Point &lb ,
                                                const Point &rt );
    bool        drawRect( const string &layer ,
                          double lbX , double lbY ,
                          double rtX , double rtY ) override;

    inline bool drawInst( const string &lib , const string &cell ,
                          const string &view ,
                          const Point &pos , double rotate );
    bool        drawInst( const string &lib , const string &cell ,
                          const string &view ,
                          double x , double y , double rotate );

  private:

    static const string GET_REPRESENT;
    static const string GET_CELLVIEW;
    static const string DRAW_RECTANGLE;
    static const string DRAW_INSTANCE;
};

inline SkillLayout::SkillLayout( const char *fileName ) : Layout( fileName ) {}

inline bool SkillLayout::drawRect(  const string    &layer ,
                                    const Rectangle &rect )
{ return Layout::drawRect( layer , rect ); }

inline bool SkillLayout::drawRect( const string &layer ,  const Point &lb ,
                                                          const Point &rt )
{ return Layout::drawRect( layer , lb , rt ); }

inline bool SkillLayout::drawInst(  const string  &lib   ,
                                    const string  &cell  ,
                                    const string  &view  ,
                                    const Point   &pos , double rotate )
{ return drawInst( lib , cell , view , pos.x() , pos.y() , rotate ); }

#endif
