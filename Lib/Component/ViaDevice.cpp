#include "ViaDevice.h"

#include <cmath>

ViaDevice::ViaDevice( Layer::Type viaLayer , double centerX , double centerY ,
                      int row , int col )
  : mCenter( Point( centerX , centerY ) ) , mRow( 0 ) , mCol( 0 ) ,
    mImpAllowed( false )
{
  setViaLayer ( viaLayer );
  setRow      ( row );
  setColumn   ( col );
  imp.setType ( Layer::UNKNOWN );
}

void ViaDevice::setViaLayer( Layer::Type viaLayer  )
{
  if( Layer::CONTACT <= viaLayer && viaLayer <= Layer::VIA56 )
  {
    via = viaLayer;
  
    switch( via )
    {
      case Layer::CONTACT:

        upper.setType( Layer::METAL1  );
        lower.setType( Layer::POLY1   );
        break;
      
      case Layer::VIA12:
      
        upper.setType( Layer::METAL2 );
        lower.setType( Layer::METAL1 );
        break;
      
      case Layer::VIA23:
      
        upper.setType( Layer::METAL3 );
        lower.setType( Layer::METAL2 );
        break;
      
      case Layer::VIA34:
      
        upper.setType( Layer::METAL4 );
        lower.setType( Layer::METAL3 );
        break;
      
      case Layer::VIA45:
      
        upper.setType( Layer::METAL5 );
        lower.setType( Layer::METAL4 );
        break;
      
      case Layer::VIA56:
      
        upper.setType( Layer::METAL6 );
        lower.setType( Layer::METAL5 );
        break;
      
      default:
      
        upper.setType( Layer::UNKNOWN );
        lower.setType( Layer::UNKNOWN );
        break;
    };
  }
  else
  {
    via = Layer::UNKNOWN;
    upper.setType( Layer::UNKNOWN );
    lower.setType( Layer::UNKNOWN );
  }
}

void ViaDevice::setRow( unsigned int row )
{
  if( row )
  {
    contacts.resize( row );
    if( row > mRow )
      for( register unsigned int i = mRow ; i < row ; i++ )
         contacts[i].resize( mCol );
  
    mRow = row;
  }
}

void ViaDevice::setColumn( unsigned int col )
{
  if( col )
  {
    for( register unsigned int i = 0 ; i < mRow ; i++ )
       contacts[i].resize( col );
    mCol = col;
  }
}
                
void ViaDevice::generate()
{
  // set contact
  double  unitCont = conWidth + conSpace;
  double  conX     = -( ( mCol - 1 ) * ( unitCont ) ) / 2;
  double  conY     = -( ( mRow - 1 ) * ( unitCont ) ) / 2;
  Layer   model;
  
  model.setType   ( via      );
  model.setWidth  ( conWidth );
  model.setHeight ( conWidth );

  for( register unsigned int i = 0 ; i < mRow ; i++ )
  {
     for( register unsigned int j = 0 ; j < mCol ; j++ )
     {
        model.setCenter( conX , conY );
        contacts[i][j] = model;
        conX += unitCont;
     }
     conY += unitCont;
     conX -= mCol * unitCont;
  }
  // end set contact
  
  // set upper layer
  double upperH = mRow * unitCont - conSpace + 2 * conInUpper.y();
  double upperW = mCol * unitCont - conSpace + 2 * conInUpper.x();

  upper.setCenter ( 0 , 0 );
  upper.setHeight ( upperH );
  upper.setWidth  ( upperW );
  // end set upper layer
  
  // set lower layer
  lower.setCenter ( 0 , 0 );
  lower.setHeight ( upperH + 2 * abs( conInLower.y() - conInUpper.y() ) );
  lower.setWidth  ( upperW + 2 * abs( conInLower.x() - conInUpper.x() ) );
  // end set lower layer
  
  // set implant
  if( lower.type() == Layer::DIFFUSION )
  {
    imp.setCenter ( 0 , 0 );
    imp.setHeight ( lower.height() + 2 * lowerInImp.y() );
    imp.setWidth  ( lower.width () + 2 * lowerInImp.x() );
  }
  // end set implant
}
