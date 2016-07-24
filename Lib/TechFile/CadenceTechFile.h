#ifndef CADENCE_TECH_FILE_H
#define CADENCE_TECH_FILE_H

#include "TechFile.h"

class CadenceTechFile : public TechFile
{
  public:

    bool read( const char *fileName ) override;

  protected:

    inline bool isNullChar( const char data ) override;

    bool findBlock() override;
    void catchWord() override;

  private:
  
    void readInfo( const string &id , void ( CadenceTechFile::*read )() );

    inline bool readBool();
};

// Cadence inline member function
inline bool CadenceTechFile::isNullChar( const char data )
{ return  data == ' ' || data == '\t'  || data == '"'; }

inline bool CadenceTechFile::readBool() { return ( word == "t" ); }
// end Cadence inline member function

#endif
