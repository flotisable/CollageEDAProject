#ifndef CIRCUIT_MODEL_H
#define CIRCUIT_MODEL_H

#include <string>
using namespace std;

#include "Model.h"
#include "../Component/Circuit.h"

class CircuitModel : public Model , public Circuit
{
  public:

    inline CircuitModel( TechFile *techFile = nullptr );
    inline CircuitModel( const string &name , TechFile *techFile = nullptr );

    inline const string& name() const;

    inline void setName( const string &name );

  private:

    string mName;
};

// CircuitModel inline member function
inline CircuitModel::CircuitModel( TechFile *techFile )
: Model( CIRCUIT ) , Circuit( techFile ) {}
inline CircuitModel::CircuitModel( const string &name , TechFile *techFile )
: Model( CIRCUIT ) , Circuit( techFile ) , mName( name ) {}

inline const string& CircuitModel::name() const { return mName; }

inline void CircuitModel::setName( const string &name ) { mName = name; }
// CircuitModel inline member function

#endif
