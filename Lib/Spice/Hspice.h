#ifndef HSPICE_H
#define HSPICE_H

#include <fstream>
#include <string>
#include <vector>
using namespace std;

class TechFile;
class SubcktModel;
class ICModel;
class Node;

class Hspice
{
  public:

    enum SubcktParam
    {
      SUBCKT_ID,
      SUBCKT_NAME,
      SUBCKT_NET
    };

    enum MParam
    {
      M_NAME,
      D,
      G,
      S,
      B,
      TYPE,
      W,
      L,
      M
    };
    
    enum XParam
    {
      X_NAME,
      X_NET
    };
    
    enum ID
    {
      VDD,
      GND,
      PMOS,
      NMOS,
      ID_NUM
    };

    Hspice( TechFile *techFile = nullptr );
    ~Hspice();

    inline SubcktModel* model() const;

    inline void setTechFile( TechFile *techFile );
    
    inline void setID( ID index , const string &name );

    bool read ( const char *fileName );
    bool write( const char *fileName = "HspiceGraph.txt" );

    void mergeModel();

  private:

    void setupModel ( int     index   );
    void setupMos   ( ICModel *model  );
    void setupSubckt( ICModel *model  );
    void setupNode  ( Node *node , ICModel *model , const string &netName );

    void  getWord     ();
    int   searchModel ( const string &name );
    
    void writeSubcktModel( SubcktModel *model );
    
    static inline bool isNullChar( char c );

    static const string SUBCKT_HEAD_KEYWORD;
    static const string SUBCKT_TAIL_KEYWORD;
    static const int    MAIN;

    TechFile              *tech;
    vector<SubcktModel*>  models;

    fstream         file;
    string          buffer;
    vector<string>  word;
    vector<string>  id;
};

inline SubcktModel* Hspice::model() const
{ return models[MAIN]; }

inline void Hspice::setTechFile( TechFile *techFile )
{ tech = techFile; }

inline void Hspice::setID( ID index , const string &name )
{ id[index] = name; }

inline bool Hspice::isNullChar( char c )
{ return ( c == ' ' || c == '\t' ); }

#endif

