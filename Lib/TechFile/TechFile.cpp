#include "TechFile.h"

TechFile::TechFile()
{
  rules.resize( SpacingRule::TYPE_NUM );
}

// public members

bool TechFile::write( const char *fileName )
{
  file.open( fileName , ios::out );
  
  if( file.is_open() )
  {
    file << left;
    file << "Tech Params:\n\n";

    for( TechParam &parameter : techParams ) file << parameter << endl;

    file << "\nSpacing Rules:\n\n";

    for( vector<SpacingRule> &specRule : rules )
       for( SpacingRule &rule : specRule ) file << rule << endl;

    file << "\nmfg Grid Resolution\n\n";
    file << mfgGridResolution << endl;
    file.close();
    return true;
  }
  return false;
}

double TechFile::param( const string &name )
{
  for( TechParam &parameter : techParams )
     if( parameter.name == name ) return parameter.value;
  return -1;
}

double TechFile::rule( SpacingRule::Type type , string layer1 ,
                                                string layer2 )
{
  for( SpacingRule &rule : rules[type] )
     if( rule.layer1 == layer1 &&
         ( layer2.empty() || rule.layer2 == layer2 ) )
       return rule.value;
  return -1;
}

// end public members

// private members

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

  catchWord();  spacingRule.type    = SpacingRule::map( word );
  catchWord();  spacingRule.layer1  = word;
  catchWord();  spacingRule.layer2  = word;
  catchWord();  spacingRule.value   = stod            ( word );

  if( spacingRule.layer2 == word )  spacingRule.layer2.clear();

  rules[spacingRule.type].push_back( spacingRule );
}

void TechFile::readOrderedSpacingRule()
{
  SpacingRule orderedSpacingRule;

  catchWord();  orderedSpacingRule.type   = SpacingRule::map( word );
  catchWord();  orderedSpacingRule.layer1 = word;
  catchWord();  orderedSpacingRule.layer2 = word;
  catchWord();  orderedSpacingRule.value  = stod            ( word );

  rules[orderedSpacingRule.type].push_back( orderedSpacingRule );
}

void TechFile::readMfgGridResolution()
{
  catchWord();  mfgGridResolution = stod( word );
}

// end private members
