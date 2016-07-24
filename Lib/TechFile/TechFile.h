#ifndef TECH_FILE_H
#define TECH_FILE_H

#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include "TechStruct.h"

class TechFile
{
  public:

    inline TechFile();
    virtual ~TechFile() = default;

    virtual bool read ( const char *fileName ) = 0;
    bool write( const char *fileName = "techInfo.txt" );

    double param( const string &name );
    double rule ( SpacingRule::Type type , Layer::Type layer1 ,
                  Layer::Type layer2 = Layer::UNKNOWN );

  protected:

    virtual inline bool isNullChar( char data );

    virtual bool findBlock() = 0;
    virtual void catchWord();

    void readTechParam          ();
    void readSpacingRule        ();
    void readOrderedSpacingRule ();
    void readMfgGridResolution  ();
    
    fstream file;
    string  word;
    string  buffer;
    
  private:

    vector<TechParam>           techParams;
    vector<vector<SpacingRule>> rules;
    double                      mfgGridResolution;
};

// TechFile inline member function
inline TechFile::TechFile()
{
  Layer::read();
  SpacingRule::read();
  rules.resize( SpacingRule::TYPE_NUM );
}

inline bool TechFile::isNullChar( char data )
{ return  data == ' ' || data == '\t'  || data == '"'; }
// end TechFile inline member function

#endif

