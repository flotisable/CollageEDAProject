#ifndef TECH_STRUCT_H
#define TECH_STRUCT_H

#include <string>
#include <ostream>
using namespace std;

// controls

struct TechParam
{
  string  name;
  double  value;
};

// end controls

// layer definitions
// end layer definitions

// layer rules
// end layer rules

// physical rules

struct SpacingRule
{
  enum Type
  {
    UNKNOWN = -1,
    MIN_WIDTH,
    MIN_SPACING,
    MIN_AREA,
    MIN_ENCLOSURE,
    MIN_NOTCH,
    DEFAULT_WIDTH,
    MAX_WIDTH,
    MIN_OVERLAP,
    TYPE_NUM,
  };

  static const string TYPES[TYPE_NUM];

  Type    type;
  string  layer1;
  string  layer2; // optional
  double  value;

  static Type                map( const string &type );
  static inline const string map( Type         type  );
};

inline const string SpacingRule::map( Type type  )
{ return ( type == UNKNOWN ) ? "unknown" : TYPES[type]; }

// end physical rules

// electrical rules
// end electrical rules

// devices
// end devices

// compactor rules
// end compactor rules

// lx rules
// end lx rules

// las rules
// end las rules

// pr rules
// end pr rules

ostream& operator<<( ostream &out , TechParam   &parameter );
ostream& operator<<( ostream &out , SpacingRule &rule );

#endif
