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

    TechFile();
    virtual ~TechFile() = default;

    virtual bool read ( const char *fileName ) = 0;
    bool write( const char *fileName = "techInfo.txt" );

    double param( const string &name );
    double rule ( SpacingRule::Type type ,  string layer1 ,
                                            string layer2 = string() );

  protected:

    virtual inline bool nullChar( const char data );

    virtual bool findBlock() = 0;
    virtual void catchWord();

    void readTechParam();

    void readSpacingRule        ();
    void readOrderedSpacingRule ();
    void readMfgGridResolution  ();
    
    fstream   file;
    string    word;
    string    buffer;

    vector<TechParam>           techParams;
    vector<vector<SpacingRule>> rules;
    double                      mfgGridResolution;
};

inline bool TechFile::nullChar( const char data )
{ return  data == ' ' || data == '\t'  || data == '"'; }

#endif

