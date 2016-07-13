#include "TechStruct.h"

#include <iomanip>

const string SpacingRule::TYPES[SpacingRule::TYPE_NUM] = {  "minWidth"    ,
                                                            "minSpacing"  ,
                                                            "minArea"     ,
                                                            "minEnclosure",
                                                            "minNotch"    ,
                                                            "defaultWidth",
                                                            "maxWidth"    ,
                                                            "minOverlap"  };

SpacingRule::Type SpacingRule::map( const string &type )
{
  for( register int i = 0 ; i < TYPE_NUM ; i++ )
     if( type == TYPES[i] ) return static_cast<Type>( i );
  return UNKNOWN;
}

ostream& operator<<( ostream &out , TechParam &parameter )
{
  const int TAB   = 10;
  const int F_TAB = out.precision();

  out << left;
  out << setw( TAB )    << parameter.name;
  out << setw( F_TAB )  << parameter.value;

  return out;
}

ostream& operator<<( ostream &out , SpacingRule &rule )
{
  const int TAB   = 15;
  const int F_TAB = out.precision();
  
  out << left;
  out << setw( TAB )    << SpacingRule::map( rule.type );
  out << setw( TAB )    << rule.layer1;
  out << setw( TAB )    << rule.layer2;
  out << setw( F_TAB )  << rule.value;
  
  return out;
}
