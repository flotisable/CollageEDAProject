#include "Layer.h"

#include <fstream>
#include <iomanip>

string Layer::TYPES[Layer::TYPE_NUM];

ostream& operator<<( ostream &out , const Layer &layer )
{
  static const int TAB = 10;

  out << left;
  out << setw( TAB ) << Layer::map( layer.type() );
  out << static_cast<Rectangle>( layer );

  return out;
}

bool Layer::read( const char *fileName )
{
  fstream file( fileName , ios::in );

  if( file.is_open() )
  {
    string buffer;

    for( string &rule : TYPES )
    {
       if( !getline( file , buffer ) ) break;

       rule = buffer.substr( buffer.rfind( ' ' ) + 1 );
    }
    return true;
  }
  return false;
}

bool Layer::write( const char *fileName )
{
  fstream file( fileName , ios::out );

  if( file.is_open() )
  {
    file << std::left;

    for( int i = 0 ; i < TYPE_NUM ; i++ )
    {
       file << setw( 14 );

       switch( i )
       {
         case DIFFUSION:  file << "DIFFUSION";  break;
         case POLY1:      file << "POLY1";      break;
         case METAL1:     file << "METAL1";     break;
         case METAL2:     file << "METAL2";     break;
         case METAL3:     file << "METAL3";     break;
         case METAL4:     file << "METAL4";     break;
         case METAL5:     file << "METAL5";     break;
         case METAL6:     file << "METAL6";     break;
         case CONTACT:    file << "CONTACT";    break;
         case VIA12:      file << "VIA12";      break;
         case VIA23:      file << "VIA23";      break;
         case VIA34:      file << "VIA34";      break;
         case VIA45:      file << "VIA45";      break;
         case VIA56:      file << "VIA56";      break;
         case NIMPLANT:   file << "NIMPLANT";   break;
         case PIMPLANT:   file << "PIMPLANT";   break;
         case NWELL:      file << "NWELL";      break;
         case PWELL:      file << "PWELL";      break;
         default:         file << "UNKNOWN";    break;
       }
       file << "= " << TYPES[i] << endl;
    }
    return true;
  }
  return false;
}

Layer::Type Layer::map( const string &type )
{
  for( unsigned int i = 0 ; i < TYPE_NUM ; i++ )
     if( TYPES[i] == type ) return static_cast<Type>( i );
  return UNKNOWN;
}
