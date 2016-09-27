#include "SkillLayout.h"

const string SkillLayout::GET_REPRESENT   = "geGetEditRep";
const string SkillLayout::GET_CELLVIEW    = "dbOpenCellViewByType";
const string SkillLayout::DRAW_RECTANGLE  = "dbCreateRect";
const string SkillLayout::DRAW_INSTANCE   = "dbCreateInst";

// SkillLayout public member function
bool SkillLayout::drawRect( Layer::Type layer , double lbX , double lbY ,
                                                double rtX , double rtY )
{
  if( rtY - lbY == 0 || rtX - lbX == 0 ) return false;

  file  << DRAW_RECTANGLE << "( "
        << GET_REPRESENT  << "() "
        << "\"" << Layer::map( layer )  << "\" "
        << "list( "
        << lbX  << ":" << lbY << " "
        << rtX  << ":" << rtY << " ) )\n";

  return static_cast<bool>( file );
}

bool SkillLayout::drawInst( const string &lib , const string &cell ,
                            const string &view ,
                            double x , double y , double rotate )
{
  file << DRAW_INSTANCE << "( "     << GET_REPRESENT  << "() "
                        << "( \""   << GET_CELLVIEW   << "\" "
                        << "\""     << lib            << "\" "
                        << "\""     << cell           << "\""
                        << " \""    << view           << "\" )"
                        << " nil "  << x              << ":" << y
                        << " \"R"   << rotate         << "\" )\n";
  return static_cast<bool>( file );
}
// end SkillLayout public member function
