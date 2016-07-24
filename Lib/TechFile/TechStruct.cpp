#include "TechStruct.h"

#include <fstream>
#include <iomanip>

string SpacingRule::TYPES[SpacingRule::TYPE_NUM];

// SpacingRule public static member function
bool SpacingRule::read( const char *fileName )
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

bool SpacingRule::write( const char *fileName )
{
  fstream file( fileName , ios::out );
  
  if( file.is_open() )
  {
    file << left;
  
    for( int i = 0 ; i < TYPE_NUM ; i++ )
    {
       file << setw( 14 );
    
       switch( i )
       {
         case MIN_WIDTH:      file << "MIN_WIDTH";      break;
         case MIN_SPACING:    file << "MIN_SPACING";    break;
         case MIN_AREA:       file << "MIN_AREA";       break;
         case MIN_ENCLOSURE:  file << "MIN_ENCLOSURE";  break;
         case MIN_NOTCH:      file << "MIN_NOTCH";      break;
         case DEFAULT_WIDTH:  file << "DEFAULT_WIDTH";  break;
         case MAX_WIDTH:      file << "MAX_WIDTH";      break;
         case MIN_OVERLAP:    file << "MIN_OVERLAP";    break;
         default:             file << "UNKNOWN";        break;
       }
       file << "= " << TYPES[i] << endl;
    }
    return true;
  }
  return false;
}

SpacingRule::Type SpacingRule::map( const string &type )
{
  for( register int i = 0 ; i < TYPE_NUM ; i++ )
     if( type == TYPES[i] ) return static_cast<Type>( i );
  return UNKNOWN;
}
// end SpacingRule public static member function

// non-member function
ostream& operator<<( ostream &out , const TechParam &parameter )
{
  const int TAB   = 10;
  const int F_TAB = out.precision();

  out << left;
  out << setw( TAB )    << parameter.name;
  out << setw( F_TAB )  << parameter.value;

  return out;
}

ostream& operator<<( ostream &out , const SpacingRule &rule )
{
  const int TAB   = 15;
  const int F_TAB = out.precision();
  
  out << left;
  out << setw( TAB )    << SpacingRule::map ( rule.type   );
  out << setw( TAB )    << Layer::map       ( rule.layer1 );
  out << setw( TAB )    << Layer::map       ( rule.layer2 );
  out << setw( F_TAB )  << rule.value;
  
  return out;
}
// end non-member function
