#ifndef TECH_STRUCT_H
#define TECH_STRUCT_H

#include <string>
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
    TYPE_NUM
  };

  static const string TYPES[TYPE_NUM];

  Type    rule;
  string  layer1;
  string  layer2; // optional
  double  value;

  static Type                map( const string &rule );
  static inline const string map( Type         rule  );
};

inline const string SpacingRule::map( Type rule  )
{ return ( rule == UNKNOWN ) ? "unknown" : TYPES[rule]; }

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

#endif
