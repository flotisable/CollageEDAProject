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

  success &= drawViaDevice( const_cast<ViaDevice*>( &mos->source () ) );
  success &= drawViaDevice( const_cast<ViaDevice*>( &mos->drain  () ) );

  success &= drawLayer( mos->implant() + center );

  return success;
}

bool Layout::drawViaDevice( ViaDevice *viaDevice )
{
  bool success = true;

  center += viaDevice->center();

  success &= drawLayer( viaDevice->lowerLayer() + center );
  success &= drawLayer( viaDevice->upperLayer() + center );

  for( unsigned int i = 0 ; i < viaDevice->row() ; i++ )
     for( unsigned int j = 0 ; j < viaDevice->column() ; j++ )
        success &= drawLayer( viaDevice->contact()[i][j] + center );

  if( viaDevice->impAllowed() )
    success &= drawLayer( viaDevice->implant() + center );

  center -= viaDevice->center();

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
  {
     for( Layer &segment : static_cast<NetNode*>( node )->segments() )
        success &= drawLayer( segment + center );
     for( ViaDevice &viaDevice : static_cast<NetNode*>( node )->contacts() )
        success &= drawViaDevice( &viaDevice );
  }

  for( Node *node : circuit->net() )
  {
     for( Layer &segment : static_cast<NetNode*>( node )->segments() )
        success &= drawLayer( segment + center );
     for( ViaDevice &viaDevice : static_cast<NetNode*>( node )->contacts() )
        success &= drawViaDevice( &viaDevice );
  }

  return success;
}
// end Layout public member function
