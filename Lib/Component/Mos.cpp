#include "Mos.h"

#include <string>
#include <fstream>
#include <iomanip>

#include "../TechFile/TechFile.h"

Mos::Mos( Type type , TechFile *techFile ) : mType( type ) , tech( techFile )
{
  diff.setType( Layer::DIFFUSION  );
  g   .setType( Layer::POLY1      );
  
  s.setViaLayer   ( Layer::CONTACT );
  s.setColumn     ( 1 );
  s.setLowerLayer ( Layer::DIFFUSION );
  
  d = s;
}

void Mos::generate()
{
  if( !tech ) return;

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

  // set viaDevice
  double  conSpace    = tech->rule( SpacingRule::MIN_SPACING   ,
                                    Layer::CONTACT );
  double  conInMetal  = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                    Layer::METAL1 , Layer::CONTACT );

  int     conNum  = 1 + ( mW - 2 * conInDiff - conWidth ) /
                    ( conWidth + conSpace );
  double  viaX    = ( mL + conWidth ) / 2 + conAndPoly;
  double  viaY    = 0;
  
  s.setCenter     ( viaX , viaY );
  s.setRow        ( conNum );
  s.setConWidth   ( conWidth );
  s.setConSpace   ( conSpace );
  s.setConInUpper ( conInMetal , 0.06 );
  s.setConInLower ( conInDiff  , conInDiff );
  s.generate      ();
  
  d = s;
  d.setCenter     ( -viaX , viaY );

  // end set viaDevice

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

    writeLayer( file , "Diffusion"   , diff           );
    writeLayer( file , "SourceMetal" , s.upperLayer() );

    for( const Layer &layer : s.contact()[0] )
       writeLayer( file , "SourceContact" , layer );

    writeLayer( file , "Gate"        , g              );
    writeLayer( file , "DrainMetal"  , d.upperLayer() );

    for( const Layer &layer : d.contact()[0] )
       writeLayer( file , "DrainContact" , layer );

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
        
        int type;
        
        file >> type;
        
        mType = static_cast<Type>( type );
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
  out << mos.type()                 << " ";
  out << setw( TAB )                << mos.w();
  out << setw( TAB )                << mos.l();
  out << mos.m()                    << endl;
  out << mos.diffusion()            << endl;
  out << mos.source().upperLayer()  << endl;

  for( const Layer &layer : mos.source().contact()[0] ) out << layer << endl;

  out << mos.gate()               << endl;
  out << mos.drain().upperLayer() << endl;

  for( const Layer &layer : mos.drain().contact()[0] )  out << layer << endl;

  return out << mos.implant() << endl;
}
