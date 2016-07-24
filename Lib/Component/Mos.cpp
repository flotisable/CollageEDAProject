#include "Mos.h"

#include <string>
#include <fstream>
#include <iomanip>

#include "../TechFile/TechFile.h"

Mos::Mos() : mType( UNKNOWN ) , tech( nullptr )
{
  diff.setType( Layer::DIFFUSION  );
  g   .setType( Layer::POLY1      );
}

Mos::Mos( int type , double w , double l , unsigned int m ,
          TechFile *techFile )
{
  mType = type;
  mW    = w;
  mL    = l;
  mM    = m;
  tech  = techFile;
  diff.setType( Layer::DIFFUSION  );
  g   .setType( Layer::POLY1      );
}

void Mos::generate()
{
  // set diffusion
  double conInDiff  = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                  Layer::DIFFUSION , Layer::CONTACT );
  double conAndPoly = tech->rule( SpacingRule::MIN_SPACING ,
                                  Layer::CONTACT , Layer::POLY1 );
  double conWidth   = tech->rule( SpacingRule::MIN_WIDTH , Layer::CONTACT  );

  diff.setCenter( 0 , 0 );
  diff.setHeight( mW );
  diff.setWidth ( mL + 2 * ( conInDiff + conWidth + conAndPoly ) );
  // end set diffusion

  // set gate
  double diffInPoly = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                  Layer::POLY1 , Layer::DIFFUSION );
  
  g.setCenter ( 0 , 0 );
  g.setHeight ( mW + 2 * diffInPoly );
  g.setWidth  ( mL );
  // end set gate

  // set metal
  double  conSpace    = tech->rule( SpacingRule::MIN_SPACING   ,
                                    Layer::CONTACT );
  double  conInMetal  = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                    Layer::METAL1 , Layer::CONTACT );

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

  model.setType   ( Layer::METAL1 );
  model.setCenter ( metalX , metalY );
  model.setHeight ( metalH );
  model.setWidth  ( metalW );
  
  s.push_back( model );
  
  model.setCenter( -metalX , metalY );
  
  d.push_back( model );
  // end set metal

  // set contact
  double  contactY = ( diff.height() - conWidth ) / 2 - conInDiff;

  model.setType   ( Layer::CONTACT );
  model.setHeight ( conWidth );
  model.setWidth  ( conWidth );

  for( register int i = 0 ; i < conNum ; i++ )
  {
     model.setCenter( -metalX , contactY ); s.push_back( model );
     model.setCenter(  metalX , contactY ); d.push_back( model );
     
     contactY -= ( conSpace + conWidth );
  }
  
  double  yBias = s[METAL].center().y() - diff.center().y();
  
  for( Layer &layer : s ) layer.setCenterY( layer.center().y() - yBias );
  for( Layer &layer : d ) layer.setCenterY( layer.center().y() - yBias );
  // end set contact

  // set implant
  Layer::Type impLayer;
  
  switch( mType )
  {
    case NMOS: impLayer = Layer::NIMPLANT;  break;
    case PMOS: impLayer = Layer::PIMPLANT;  break;
    default:                                break;
  }
  
  double DiffInImp = tech->rule(  SpacingRule::MIN_ENCLOSURE ,
                                  impLayer , Layer::DIFFUSION );

  imp.setType   ( impLayer );
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
  const double TAB = out.precision();

  out << left;
  out << mos.type()               << " ";
  out << setw( TAB )              << mos.w();
  out << setw( TAB )              << mos.l();
  out << mos.m()                  << endl;
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
