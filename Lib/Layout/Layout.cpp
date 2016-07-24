#include "Layout.h"

#include "../Component/Circuit.h"
#include "../Component/Mos.h"
#include "../Component/ViaDevice.h"
#include "../Model/MosModel.h"
#include "../Model/CircuitModel.h"
#include "../Node/MosNode.h"
#include "../Node/CircuitNode.h"
#include "../Node/NetNode.h"

// Layout public member function
bool Layout::drawMos( Mos *mos )
{
  bool  success = true;

  success &= drawLayer( mos->diffusion() + center );
  success &= drawLayer( mos->gate     () + center );

  for( auto &layer : mos->source() ) success &= drawLayer( layer + center );
  for( auto &layer : mos->drain () ) success &= drawLayer( layer + center );

  success &= drawLayer( mos->implant() + center );

  return success;
}

bool Layout::drawViaDevice( ViaDevice *viaDevice )
{
  bool success = true;

  success &= drawLayer( viaDevice->diffusion() + center );
  success &= drawLayer( viaDevice->metal    () + center );

  for( int i = 0 ; i < viaDevice->row() ; i++ )
     for( int j = 0 ; j < viaDevice->column() ; j++ )
        success &= drawLayer( viaDevice->contact()[i][j] + center );

  success &= drawLayer( viaDevice->implant() + center );

  return success;
}

bool Layout::drawCircuit( Circuit *circuit )
{
  bool success = true;

  for( Node *mosNode : circuit->mosCell() )
  {
     MosNode *node = static_cast<MosNode*>( mosNode );

     center   += node->center();
     success  &= drawMos( static_cast<MosModel*>( node->model() ) );
     center   -= node->center();
  }

  for( Node *circuitNode : circuit->circuitCell() )
  {
     CircuitNode *node = static_cast<CircuitNode*>( circuitNode );

     success  &= drawRect( Layer::NWELL , *node + center );

     center   += node->center ();
     success  &= drawCircuit  ( static_cast<CircuitModel*>( node->model() ) );
     center   -= node->center ();
  }

  for( Node *node : circuit->io() )
     for( Layer &segment : static_cast<NetNode*>( node )->segments() )
        success &= drawLayer( segment + center );

  for( Node *node : circuit->net() )
     for( Layer &segment : static_cast<NetNode*>( node )->segments() )
        success &= drawLayer( segment + center );

  return success;
}
// end Layout public member function
