#include "SkillLayout.h"

#include "../Model/MosModel.h"
#include "../Component/Mos.h"
#include "../Component/ViaDevice.h"
#include "../Model/SubcktModel.h"
#include "../Model/ICModel.h"
#include "../Node/MosNode.h"
#include "../Node/SubcktNode.h"
#include "../Node/NetNode.h"

const string SkillLayout::GET_REPRESENT   = "geGetEditRep";
const string SkillLayout::GET_CELLVIEW    = "dbOpenCellViewByType";
const string SkillLayout::DRAW_RECTANGLE  = "dbCreateRect";
const string SkillLayout::DRAW_INSTANCE   = "dbCreateInst";

bool SkillLayout::drawRect( const string &layer , double lbX , double lbY ,
                                                  double rtX , double rtY )
{
  if( rtY - lbY == 0 || rtX - lbX == 0 ) return false;

  return file << DRAW_RECTANGLE << "( "
              << GET_REPRESENT  << "() "
              << "\"" << layer  << "\" "
              << "list( "       << lbX  << ":" << lbY << " "
                                << rtX  << ":" << rtY << " ) )\n";
}

bool SkillLayout::drawInst( const string &lib , const string &cell ,
                            const string &view ,
                            double x , double y , double rotate )
{
  return file << DRAW_INSTANCE  << "( "     << GET_REPRESENT  << "() "
                                << "( \""   << GET_CELLVIEW   << "\" "
                                << "\""     << lib            << "\" "
                                << "\""     << cell           << "\""
                                << " \""    << view           << "\" )"
                                << " nil "  << x              << ":" << y
                                << " \"R"   << rotate         << "\" )\n";
}

bool SkillLayout::drawMos( Mos *mos )
{
  bool  success = true;

  success &= drawLayer( mos->diffusion()             + center );
  success &= drawLayer( mos->gate     ()             + center );
  
  for( unsigned int i = 0 ; i < mos->source().size() ; i++ )
  {
     success &= drawLayer( mos->source()[i] + center );
     success &= drawLayer( mos->drain ()[i] + center );
  }
  
  success &= drawLayer( mos->implant() + center );
  
  return success;
}

bool SkillLayout::drawViaDevice( ViaDevice *viaDevice )
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

bool SkillLayout::drawSubckt( ICModel *subckt )
{
  bool success                = true;
  vector<Node*> &mosCells     = subckt->mosCell   ();
  vector<Node*> &subcktCells  = subckt->subcktCell();
  vector<Node*> &nets         = subckt->net       ();
  vector<Node*> &ios          = subckt->io        ();

  for( unsigned int i = 0 ; i < mosCells.size() ; i++ )
  {
     MosNode  *node     = static_cast<MosNode*> ( mosCells[i]   );
     
     center   += node->center();
     success  &= drawMos( static_cast<MosModel*>( node->model() )->model() );
     center   -= node->center();
  }

  for( unsigned int i = 0 ; i < subcktCells.size() ; i++ )
  {
     SubcktNode   *node   = static_cast<SubcktNode*>  ( subcktCells[i]  );
     SubcktModel  *model  = static_cast<SubcktModel*> ( node->model()   );

     success  &= drawRect   ( "NWELL" , static_cast<Rectangle>( *node )
                                        + center );

     center   += node->center ();
     success  &= drawSubckt   ( model->model() );
     center   -= node->center ();
  }

  for( unsigned int i = 0 ; i < ios.size() ; i++ )
  {
     vector<Layer>  &netlist    = static_cast<NetNode*>( ios[i] )->nets();
     //string         layer;

     for( unsigned int j = 0 ; j < netlist.size() ; j++ )
     /*{
        if      ( netlist[j].height() > netlist[j].width() )
          layer = "METAL1";
        else if ( netlist[j].height() == netlist[j].width() )
        {
          if      ( netlist[j].height() == 0.22 ) layer = "CONT";
          else if ( netlist[j].height() == 0.26 ) layer = "VIA12";
          else                                    layer = "METAL1";
        }
        else
          layer = "METAL2";*/

        success &= drawLayer( netlist[j] + center );
     //}
  }

  for( unsigned int i = 0 ; i < nets.size() ; i++ )
  {
     vector<Layer>  &netlist = static_cast<NetNode*>( nets[i] )->nets();
     //string         layer;
     
     for( unsigned int j = 0 ; j < netlist.size() ; j++ )
     /*{
        if      ( netlist[j].height() > netlist[j].width() )
          layer = "METAL1";
        else if ( netlist[j].height() == netlist[j].width() )
        {
          if      ( netlist[j].height() == 0.22 ) layer = "CONT";
          else if ( netlist[j].height() == 0.26 ) layer = "VIA12";
          else                                    layer = "METAL1";
        }
        else
          layer = "METAL2";*/
        
        success &= drawLayer( netlist[j] + center );
     //}
  }

  return success;
}
