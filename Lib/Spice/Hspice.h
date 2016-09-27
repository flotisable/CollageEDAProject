#ifndef HSPICE_H
#define HSPICE_H

#include <fstream>
#include <string>
#include <vector>
using namespace std;

class TechFile;
class CircuitModel;
class Circuit;
class Node;
class CircuitBoard;

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

    Hspice();

    inline void setCircuitBoard ( CircuitBoard *board );
    inline void setID           ( ID index , const string &name );

    bool read ( const char *fileName );
    bool write( const char *fileName = "HspiceGraph.txt" );

  private:

    void setupModel ( Circuit *model );
    void setupMos   ( Circuit *model );
    void setupSubckt( Circuit *model );
    void setupNode  ( Node *node , Circuit *model , const string &netName );

    void getWord();
    
    void writeCircuitModel( CircuitModel *model );
    
    static inline bool isNullChar( char c );

    static const string SUBCKT_HEAD_KEYWORD;
    static const string SUBCKT_TAIL_KEYWORD;

    CircuitBoard  *circuitBoard;

    fstream         file;
    string          buffer;
    vector<string>  word;
    vector<string>  id;
};

inline void Hspice::setCircuitBoard( CircuitBoard *board )
{ circuitBoard = board; }

inline void Hspice::setID( ID index , const string &name )
{ id[index] = name; }

inline bool Hspice::isNullChar( char c )
{ return ( c == ' ' || c == '\t' || c == '\r' ); }

#endif

