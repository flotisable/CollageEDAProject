#include "Mos.h"

#include <string>
#include <fstream>
#include <iomanip>

#include "../TechFile/TechFile.h"

Mos::Mos() : mType( UNKNOWN ) , tech( nullptr )
{
  diff.setLayer( "DIFF"   );
  g   .setLayer( "POLY1"  );
}

Mos::Mos( int type , double w , double l , unsigned int m ,
          TechFile *techFile )
{
  mType = type;
  mW    = w;
  mL    = l;
  mM    = m;
  tech  = techFile;
  diff.setLayer( "DIFF"   );
  g   .setLayer( "POLY1"  );
}

void Mos::generate()
{
  // set diffusion
  double conInDiff  = tech->rule( SpacingRule::MIN_ENCLOSURE ,  "DIFF" ,
                                                                "CONT"  );
  double conAndPoly = tech->rule( SpacingRule::MIN_SPACING   ,  "CONT" ,
                                                                "POLY1" );
  double conWidth   = tech->rule( SpacingRule::MIN_WIDTH     ,  "CONT"  );

  diff.setCenter( 0 , 0 );
  diff.setHeight( mW );
  diff.setWidth ( mL + 2 * ( conInDiff + conWidth + conAndPoly ) );
  // end set diffusion

  // set gate
  double diffInPoly = tech->rule( SpacingRule::MIN_ENCLOSURE ,  "POLY1" ,
                                                                "DIFF"  );
  
  g.setCenter ( 0 , 0 );
  g.setHeight ( mW + 2 * diffInPoly );
  g.setWidth  ( mL );
  // end set gate

  // set metal
  double  conSpace    = tech->rule( SpacingRule::MIN_SPACING   ,  "CONT"  );
  double  conInMetal  = tech->rule( SpacingRule::MIN_ENCLOSURE ,  "METAL1" ,
                                                                  "CONT"  );

  int     conNum  = 1 + ( mW - 2 * conInDiff - conWidth ) /
                    ( conWidth + conSpace );
  double  metalH  = conNum * ( conWidth + conSpace ) -
                    conSpace + 2 * 0.06/*conInMetal*/;
  double  metalW  = conWidth  + 2 * conInMetal;
  double  metalX  = ( mL + conWidth  ) / 2 + conAndPoly;
  double  metalY  = ( mW - metalH    ) / 2 - conInDiff   +
                     0.06/*conInMetal*/;

  s.clear();
  d.clear();

  Layer model;

  model.setLayer  ( "METAL1" );
  model.setCenter ( metalX , metalY );
  model.setHeight ( metalH );
  model.setWidth  ( metalW );
  
  s.push_back( model );
  
  model.setCenter( -metalX , metalY );
  
  d.push_back( model );
  // end set metal

  // set contact
  double  contactY = ( diff.height() - conWidth ) / 2 - conInDiff;

  model.setLayer  ( "CONT" );
  model.setHeight ( conWidth );
  model.setWidth  ( conWidth );

  for( register int i = 0 ; i < conNum ; i++ )
  {
     model.setCenter( -metalX , contactY ); s.push_back( model );
     model.setCenter(  metalX , contactY ); d.push_back( model );
     
     contactY -= ( conSpace + conWidth );
  }
  // end set contact

  // set implant
  string impLayer;
  
  switch( mType )
  {
    case NMOS: impLayer = "NIMP"; break;
    case PMOS: impLayer = "PIMP"; break;
    default:                      break;
  }
  
  double DiffInImp = tech->rule( SpacingRule::MIN_ENCLOSURE , impLayer ,
                                                              "DIFF" );

  imp.setLayer  ( impLayer );
  imp.setCenter ( 0 , 0 );
  imp.setHeight ( diff.height () + 2 * 0.35/*DiffInImp*/ );
  imp.setWidth  ( diff.width  () + 2 * DiffInImp );
  // end set implant
}

bool Mos::write( const char *fileName )
{
  fstream file( fileName , ios::out );

  if( file.is_open() )
  {
    const int TAB = file.precision();
  
    file << left;
    file << setw( 14 )              << "layer";
    file << setw( 2 * TAB + 7 + 1 ) << "center";
    file << setw( TAB + 1 )         << "height";
    file << setw( TAB + 1 )         << "width";
    file << endl;

    writeLayer( file , "Diffusion"   , diff     );
    writeLayer( file , "SourceMetal" , s[METAL] );

    for( unsigned int i = CONTACT ; i < s.size() ; i++ )
       writeLayer( file , "SourceContact" , s[i] );

    writeLayer( file , "Gate"        , g        );
    writeLayer( file , "DrainMetal"  , d[METAL] );

    for( unsigned int i = CONTACT ; i < d.size() ; i++ )
       writeLayer( file , "DrainContact" , d[i] );

    writeLayer( file , "Implant"     , imp      );

    file.close();
    return true;
  }
  return false;
}

bool Mos::read( const char *fileName )
{
  fstream file( fileName , ios::in );
  string  id;
  
  if( file.is_open() )
  {
    while( file >> id )
    {
      if      ( id == "Type" )
      {
        while( id != ":" )  file >> id;
        file >> mType;
      }
      else if ( id == "W" )
      {
        while( id != ":" )  file >> id;
        file >> mW;
      }
      else if ( id == "L" )
      {
        while( id != ":" )  file >> id;
        file >> mL;
      }
      else if ( id == "M" )
      {
        while( id != ":" )  file >> id;
        file >> mM;
      }
    }

    file.close();
    return true;
  }
  return false;
}


void Mos::writeLayer( ostream &output , const char *name ,
                      const Layer &layer )
{
  output << setw( 14 ) << name;
  output << static_cast<Rectangle>( layer );
  output << endl;
}


ostream& operator<<( ostream &out , Mos &mos )
{
  out << mos.diffusion()          << endl;
  out << mos.source()[Mos::METAL] << endl;

  for( unsigned int i = Mos::CONTACT ; i < mos.source().size() ; i++ )
     out << mos.source()[i] << endl;

  out << mos.gate()              << endl;
  out << mos.drain()[Mos::METAL] << endl;

  for( unsigned int i = Mos::CONTACT ; i < mos.drain().size() ; i++ )
     out << mos.drain()[i] << endl;

  return out << mos.implant() << endl;
}
