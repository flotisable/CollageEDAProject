#ifndef CIRCUIT_MODEL_H
#define CIRCUIT_MODEL_H

#include <string>
using namespace std;

#include "Model.h"
#include "../Component/Circuit.h"

class ICPlacement;
class ICRouting;

class CircuitModel : public Model , public Circuit
{
  public:

    inline CircuitModel(  TechFile    *techFile   = nullptr ,
                          ICPlacement *placement  = nullptr ,
                          ICRouting   *routing    = nullptr );
    inline CircuitModel(  const string  &name ,
                          TechFile      *techFile   = nullptr ,
                          ICPlacement   *placement  = nullptr ,
                          ICRouting     *routing    = nullptr );

    inline const string& name() const;

    inline void setPlacement( ICPlacement   *placement  );
    inline void setRouting  ( ICRouting     *routing    );
    inline void setName     ( const string  &name       );
    
    bool layout();

  private:

    ICPlacement *placer;
    ICRouting   *router;

    string mName;
};

// CircuitModel inline member function
inline CircuitModel::CircuitModel(  TechFile  *techFile ,
                                    ICPlacement *placement ,
                                    ICRouting *routing )
  : Model( CIRCUIT ) , Circuit( techFile ) , placer( placement ) ,
    router( routing ) {}
inline CircuitModel::CircuitModel(  const string  &name , TechFile *techFile ,
                                    ICPlacement   *placement ,
                                    ICRouting     *routing )
  : Model( CIRCUIT ) , Circuit( techFile ) , placer( placement ) ,
    router( routing ) , mName( name ) {}

inline const string& CircuitModel::name() const { return mName; }

inline void CircuitModel::setPlacement( ICPlacement  *placement )
{ placer  = placement;  }
inline void CircuitModel::setRouting  ( ICRouting    *routing   )
{ router  = routing;    }
inline void CircuitModel::setName     ( const string &name      )
{ mName   = name;       }
// CircuitModel inline member function

#endif
