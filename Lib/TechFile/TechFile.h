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

    static const int TAB;
    
    TechFile();

    bool read ( const char *fileName );
    bool write( const char *fileName = "techInfo.txt" );

    double param( const string &name );
    double rule ( SpacingRule::Type rule ,  string layer1 ,
                                            string layer2 = string() );

  private:

    fstream   file;
    string    word;
    string    buffer;

    vector<TechParam>           techParams;
    vector<vector<SpacingRule>> rules;
    double                      mfgGridResolution;

    static inline bool nullChar( const char data );

    bool findBlock();
    void readInfo ( const string &id , void ( TechFile::*read )() );
    void catchWord();

    inline bool readBool  ();

    // controls
    void readTechParam();
    // end controls

    // layer definitions
    // end layer definitions

    // layer rules
    // end layer rules

    // physical rules
    void readSpacingRule        ();
    void readOrderedSpacingRule ();
    void readMfgGridResolution  ();
    // end physical rules

    // electrical rules
    // end electrical rules

    // devices
    // end devices

    // virtuoso compactor rules
    // end virtuoso compactor rules

    // virtuoso XL rules
    // end virtuoso XL rules

    // virtuoso layout synthesizer rules
    // end virtuoso layout synthesizer rules

    // place and route rules
    // end place and route rules
};

inline bool TechFile::nullChar( const char data )
{ return  data == ' ' || data == '\t'  || data == '"'; }

inline bool TechFile::readBool() { return ( word == "t" ); }

#endif

