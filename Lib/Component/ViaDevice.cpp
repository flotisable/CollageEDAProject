#include "ViaDevice.h"

#include "../TechFile/TechFile.h"

ViaDevice::ViaDevice( const string &viaLayer ,
                      double centerX , double centerY ,
                      int row , int col , TechFile *techFile )
{
  via       = viaLayer;
  m_center  = Point( centerX , centerY );
  m_row     = m_col = 0;
  setRow    ( row );
  setColumn ( col );
  tech      = techFile;
}

void ViaDevice::setRow( unsigned int row )
{
  if( row )
  {
    contacts.resize( row );
    if( row > m_row )
      for( register int i = m_row ; i < row ; i++ )
         contacts[i].resize( m_col );
  
    m_row = row;
  }
}

void ViaDevice::setColumn( unsigned int col )
{
  if( col )
  {
    for( register int i = 0 ; i < m_row ; i++ )
       contacts[i].resize( col );
    m_col = col;
  }
}
                
void ViaDevice::generate()
{
  // set contact
  double    conWidth = tech->rule( SpacingRule::MIN_WIDTH    , via );
  double    conSpace = tech->rule( SpacingRule::MIN_SPACING  , via );
  double    unitCont = conWidth + conSpace;
  double    conX     = -( ( m_col - 1 ) * ( unitCont ) ) / 2;
  double    conY     =  ( ( m_row - 1 ) * ( unitCont ) ) / 2;
  Rectangle model;
  
  model.setWidth ( conWidth );
  model.setHeight( conWidth );

  for( register int i = 0 ; i < m_row ; i++ )
  {
     for( register int j = 0 ; j < m_col ; j++ )
     {
        model.setCenter( conX , conY );
        contacts[i][j] = model;
        conX += unitCont;
     }
     conY -= unitCont;
     conX -= ( m_col - 1 ) * unitCont;
  }
  // end set contact
  
  // set metal
  double conInMetal = tech->rule( SpacingRule::MIN_ENCLOSURE ,  "METAL1" ,
                                                                via );
  double metalH     = m_row * unitCont - conSpace + 2 * conInMetal;
  double metalW     = m_col * unitCont - conSpace + 2 * conInMetal;
  
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
  
  switch( m_type )
  {
    case N:   impLayer = "NIMP";  break;
    case P:   impLayer = "PIMP";  break;
    default:                      break;
  }
  
  double  diffInImp = tech->rule( SpacingRule::MIN_ENCLOSURE ,  impLayer ,
                                                                "DIFF" );
  
  imp.setCenter ( 0 , 0 );
  imp.setHeight ( diff.height () + 2 * diffInImp );
  imp.setWidth  ( diff.width  () + 2 * diffInImp );
  // end set implant
}
