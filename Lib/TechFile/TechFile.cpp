#include "TechFile.h"
#include <limits>
#include <iomanip>

const int TechFile::TAB = 15;

TechFile::TechFile()
{
  rules.resize( SpacingRule::TYPE_NUM );
}

// public members

bool TechFile::read( const char *fileName )
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

    readInfo( "techParams"          , &TechFile::readTechParam          );
    readInfo( "spacingRules"        , &TechFile::readSpacingRule        );
    readInfo( "orderedSpacingRules" , &TechFile::readOrderedSpacingRule );
    readInfo( "mfgGridResolution"   , &TechFile::readMfgGridResolution  );
  }

  file.close();
  return true;
}

bool TechFile::write( const char *fileName )
{
  file.open( fileName , ios::out );
  
  if( file.is_open() )
  {
    file << left;
    file << "Tech Params:\n\n";

    for( register unsigned int i = 0 ; i < techParams.size() ; i++ )
    {
       file << setw( TAB ) << techParams[i].name;
       file << setw( TAB ) << techParams[i].value;
       file << endl;
    }

    file << "\nSpacing Rules:\n\n";

    for( register unsigned int i = 0 ; i < rules.size() ; i++ )
       for( register unsigned int j = 0 ; j < rules[i].size() ; j++ )
       {
          file << setw( TAB ) << SpacingRule::map( rules[i][j].rule );
          file << setw( TAB ) << rules[i][j].layer1;
          file << setw( TAB ) << rules[i][j].layer2;
          file << setw( TAB ) << rules[i][j].value;
          file << endl;
       }

    file << "\nmfg Grid Resolution\n\n";
    file << mfgGridResolution << endl;
    file.close();
    return true;
  }
  return false;
}

double TechFile::param( const string &name )
{
  for( register unsigned int i = 0 ; i < techParams.size() ; i++ )
     if( techParams[i].name == name ) return techParams[i].value;
  return -1;
}

double TechFile::rule( SpacingRule::Type rule , string layer1 ,
                                                string layer2 )
{
  for( register unsigned int i = 0 ; i < rules[rule].size() ; i++ )
     if( rules[rule][i].layer1 == layer1 &&
         ( layer2.empty() || rules[rule][i].layer2 == layer2 ) )
       return rules[rule][i].value;
  return -1;
}

// end public members

// private members

bool TechFile::findBlock()
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

void TechFile::readInfo( const string &id , void (TechFile::*read)() )
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

void TechFile::catchWord()
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

void TechFile::readTechParam()
{
  TechParam techParam;

  catchWord();  techParam.name  = word;
  catchWord();  techParam.value = stod( word );

  techParams.push_back( techParam );
}


void TechFile::readSpacingRule()
{
  SpacingRule spacingRule;

  catchWord();  spacingRule.rule    = SpacingRule::map( word );
  catchWord();  spacingRule.layer1  = word;
  catchWord();  spacingRule.layer2  = word;
  catchWord();  spacingRule.value   = stod            ( word );

  if( spacingRule.layer2 == word )  spacingRule.layer2.clear();

  rules[spacingRule.rule].push_back( spacingRule );
}

void TechFile::readOrderedSpacingRule()
{
  SpacingRule orderedSpacingRule;

  catchWord();  orderedSpacingRule.rule   = SpacingRule::map( word );
  catchWord();  orderedSpacingRule.layer1 = word;
  catchWord();  orderedSpacingRule.layer2 = word;
  catchWord();  orderedSpacingRule.value  = stod            ( word );

  rules[orderedSpacingRule.rule].push_back( orderedSpacingRule );
}

void TechFile::readMfgGridResolution()
{
  catchWord();  mfgGridResolution = stod( word );
}

// end private members
