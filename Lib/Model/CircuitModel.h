#ifndef CIRCUIT_MODEL_H
#define CIRCUIT_MODEL_H

#include <string>
using namespace std;

#include "Model.h"
#include "../Component/Circuit.h"

class CircuitModel : public Model , public Circuit
{
  public:

    inline CircuitModel();

    inline const string& name() const;

    inline void setName( const string &name );

  private:

    string mName;
};

// CircuitModel inline member function
inline CircuitModel::CircuitModel() : Model( CIRCUIT ) {}

inline const string& CircuitModel::name() const { return mName;   }

inline void CircuitModel::setName( const string &name ) { mName = name; }
// CircuitModel inline member function

#endif
