#include "Layout.h"

#include "../Component/Mos.h"
#include "../Component/ViaDevice.h"
#include "../Model/MosModel.h"
#include "../Model/SubcktModel.h"
#include "../Model/ICModel.h"
#include "../Node/MosNode.h"
#include "../Node/SubcktNode.h"
#include "../Node/NetNode.h"

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

bool Layout::drawSubckt( ICModel *subckt )
{
  bool success = true;

  for( Node *mosNode : subckt->mosCell() )
  {
     MosNode  *node = static_cast<MosNode*>( mosNode );

     center   += node->center();
     success  &= drawMos( static_cast<MosModel*>( node->model() ) );
     center   -= node->center();
  }

  for( Node *subcktNode : subckt->subcktCell() )
  {
     SubcktNode   *node   = static_cast<SubcktNode*>  ( subcktNode    );

     success  &= drawRect(  "NWELL" ,
                            static_cast<Rectangle>( *node ) + center );

     center   += node->center ();
     success  &= drawSubckt   ( static_cast<SubcktModel*>( node->model() ) );
     center   -= node->center ();
  }

  for( Node *node : subckt->io() )
     for( Layer &layer : static_cast<NetNode*>( node )->nets() )
        success &= drawLayer( layer + center );

  for( Node *node : subckt->net() )
     for( Layer &layer : static_cast<NetNode*>( node )->nets() )
        success &= drawLayer( layer + center );

  return success;
}
