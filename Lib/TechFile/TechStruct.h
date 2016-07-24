#ifndef TECH_STRUCT_H
#define TECH_STRUCT_H

#include <string>
#include <ostream>
using namespace std;

#include "../Component/Layer.h"

// controls

struct TechParam
{
  string  name;
  double  value;
};

// end controls

// physical rules

struct SpacingRule
{
  enum Type
  {
    MIN_WIDTH,
    MIN_SPACING,
    MIN_AREA,
    MIN_ENCLOSURE,
    MIN_NOTCH,
    DEFAULT_WIDTH,
    MAX_WIDTH,
    MIN_OVERLAP,
    TYPE_NUM,
    UNKNOWN
  };

  static inline void setTypesName( Type type , const string &name );

  static bool read  ( const char *fileName = "spacingRuleTypes.txt" );
  static bool write ( const char *fileName = "spacingRuleTypes.txt" );

  static Type           map( const string &type );
  static inline string  map( Type         type  );

  Type        type;
  Layer::Type layer1;
  Layer::Type layer2; // optional
  double      value;
  
  private:
  
    static string TYPES[TYPE_NUM];
};

inline void SpacingRule::setTypesName( Type type , const string &name )
{
  if( type == UNKNOWN ) return;
  else                  TYPES[type] = name;
}

inline string SpacingRule::map( Type type  )
{ return ( type == UNKNOWN ) ? "unknown" : TYPES[type]; }

// end physical rules

// non-member function
ostream& operator<<( ostream &out , const TechParam   &parameter  );
ostream& operator<<( ostream &out , const SpacingRule &rule       );
// end non-member function

#endif
