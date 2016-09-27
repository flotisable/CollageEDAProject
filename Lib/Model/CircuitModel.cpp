#include "CircuitModel.h"

#include "../EDA/ICPlacement.h"
#include "../EDA/ICRouting.h"

bool CircuitModel::layout()
{
  if( !tech || !placer || !router ) return false;

  generate();
  placer->setTechFile( tech );
  router->setTechFile( tech );

  bool success = true;

  for( Model *circuitModel : this->circuitModel() )
  {
     CircuitModel *model = static_cast<CircuitModel*>( circuitModel );

     success &= placer->placement ( model );
     success &= router->routing   ( model );

     if( !success ) return false;
  }

  success &= placer->placement( this );
  success &= router->routing  ( this );

  return success;
}
