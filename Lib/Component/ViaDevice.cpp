#include "ViaDevice.h"

#include "../TechFile/TechFile.h"

ViaDevice::ViaDevice( const string &viaLayer ,
                      double centerX , double centerY ,
                      int row , int col , TechFile *techFile )
{
  via       = viaLayer;
  mCenter  = Point( centerX , centerY );
  mRow     = mCol = 0;
  setRow    ( row );
  setColumn ( col );
  tech      = techFile;
  diff.setLayer( "DIFF" );
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
  double    conWidth = tech->rule( SpacingRule::MIN_WIDTH    , via );
  double    conSpace = tech->rule( SpacingRule::MIN_SPACING  , via );
  double    unitCont = conWidth + conSpace;
  double    conX     = -( ( mCol - 1 ) * ( unitCont ) ) / 2;
  double    conY     =  ( ( mRow - 1 ) * ( unitCont ) ) / 2;
  Layer model;
  
  model.setLayer ( via      );
  model.setWidth ( conWidth );
  model.setHeight( conWidth );

  for( register unsigned int i = 0 ; i < mRow ; i++ )
  {
     for( register unsigned int j = 0 ; j < mCol ; j++ )
     {
        model.setCenter( conX , conY );
        contacts[i][j] = model;
        conX += unitCont;
     }
     conY -= unitCont;
     conX -= ( mCol - 1 ) * unitCont;
  }
  // end set contact
  
  // set metal
  double conInMetal = tech->rule( SpacingRule::MIN_ENCLOSURE ,  "METAL1" ,
                                                                via );
  double metalH     = mRow * unitCont - conSpace + 2 * conInMetal;
  double metalW     = mCol * unitCont - conSpace + 2 * conInMetal;
  
  m.setLayer  ( "METAL1" );
  m.setCenter ( 0 , 0 );
  m.setHeight ( metalH );
  m.setWidth  ( metalW );
  // end set metal
  
  // set diffusion
  double conInDiff = tech->rule( SpacingRule::MIN_ENCLOSURE , "DIFF" ,
                                                              via );
  
  diff.setCenter ( 0 , 0 );
  diff.setHeight ( metalH + 2 * ( conInDiff - conInMetal ) );
  diff.setWidth  ( metalW + 2 * ( conInDiff - conInMetal ) );
  // end set diffusion
  
  // set implant
  string impLayer;
  
  switch( mType )
  {
    case N:   impLayer = "NIMP";  break;
    case P:   impLayer = "PIMP";  break;
    default:                      break;
  }
  
  double  diffInImp = tech->rule( SpacingRule::MIN_ENCLOSURE ,  impLayer ,
                                                                "DIFF" );
  
  imp.setLayer  ( impLayer );
  imp.setCenter ( 0 , 0 );
  imp.setHeight ( diff.height () + 2 * diffInImp );
  imp.setWidth  ( diff.width  () + 2 * diffInImp );
  // end set implant
}
