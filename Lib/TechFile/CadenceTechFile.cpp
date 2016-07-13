#include "CadenceTechFile.h"
#include <limits>

// public members

bool CadenceTechFile::read( const char *fileName )
{
  file.open( fileName , ios::in );

  while( file >> buffer )
  {
    switch( buffer[0] )
    {
      case ';':
      case ')':

        file.ignore( numeric_limits<streamsize>::max() , '\n' );
        continue;

      case '(':

        int   layer = 1;
        char  c;

        while( file >> c )
        {
          switch( c )
          {
            case '(': layer++; break;
            case ')': layer--; break;
          }
          if( layer == 0 ) break;
        }

        if( layer ) return false;
    }

    if( !findBlock() ) return false;

    readInfo( "techParams"          , &CadenceTechFile::readTechParam          );
    readInfo( "spacingRules"        , &CadenceTechFile::readSpacingRule        );
    readInfo( "orderedSpacingRules" ,
              &CadenceTechFile::readOrderedSpacingRule );
    readInfo( "mfgGridResolution"   ,
              &CadenceTechFile::readMfgGridResolution  );
  }

  file.close();
  return true;
}

// end public members

// private members

bool CadenceTechFile::findBlock()
{
  unsigned int  startIndex = buffer.find( "(" );
  char          c;

  buffer.erase( startIndex );
  word = buffer;

  if( startIndex == string::npos )
  {
    while( file >> c )
      if( c == '(' )
        return true;
    return false;
  }
  return true;
}

void CadenceTechFile::catchWord()
{
  for( register unsigned int i = 0 ; i < buffer.length() ; i++ )
  {
     if( nullChar( buffer[i] ) ) continue;

     for( register unsigned int j = i ; j < buffer.length() ; j++ )
        if( nullChar( buffer[j] ) )
        {
          word = buffer.substr( i , j - i );
          buffer.erase( 0 , j + 1 );
          return;
        }

     word = buffer.substr( i );
     break;
  }
  buffer.clear();
}

void CadenceTechFile::readInfo( const string &id ,
                                void (CadenceTechFile::*read)() )
{
  char  c;

  if( word == id )
    while( file >> c )
      switch( c )
      {
        case ';':

          file.ignore( numeric_limits<streamsize>::max() , '\n' );
          break;

        case '(':

          unsigned int  endIndex;
          unsigned int  commentIndex;

          getline( file , buffer );
          commentIndex  = buffer.find( ';' );
          endIndex      = buffer.rfind( ')' );
          if( commentIndex != string::npos )
            while( endIndex > commentIndex )
              endIndex = buffer.rfind( ')' , endIndex );

          buffer.erase( endIndex );
          ( this->*read )();
          break;

        case ')': return;
      }
}

// end private members
