#ifndef CADENCE_TECH_FILE_H
#define CADENCE_TECH_FILE_H

#include "TechFile.h"

class CadenceTechFile : public TechFile
{
  public:

    bool read( const char *fileName ) override;

  protected:

    inline bool nullChar( const char data ) override;

    bool findBlock() override;
    void catchWord() override;

  private:
  
    void readInfo( const string &id , void ( CadenceTechFile::*read )() );

    inline bool readBool();
};

inline bool CadenceTechFile::nullChar( const char data )
{ return  data == ' ' || data == '\t'  || data == '"'; }

inline bool CadenceTechFile::readBool() { return ( word == "t" ); }

#endif
