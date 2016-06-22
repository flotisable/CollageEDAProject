#include "TechStruct.h"

const string SpacingRule::TYPES[SpacingRule::TYPE_NUM] = {  "minWidth"    ,
                                                            "minSpacing"  ,
                                                            "minArea"     ,
                                                            "minEnclosure",
                                                            "minNotch"    ,
                                                            "defaultWidth",
                                                            "maxWidth"    ,
                                                            "minOverlap"  };

SpacingRule::Type SpacingRule::map( const string &rule )
{
  for( register int i = 0 ; i < TYPE_NUM ; i++ )
     if( rule == TYPES[i] ) return static_cast<Type>( i );
  return UNKNOWN;
}
