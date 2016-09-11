#ifndef IC_ROUTING_H
#define IC_ROUTING_H

#include <vector>

class CircuitModel;
class TechFile;
class NetNode;

struct Block;

class ICRouting
{
  public:

    inline ICRouting( TechFile *techFile = nullptr );

    inline void setTechFile( TechFile *techFile );

    bool routing( CircuitModel *model );

  private:

    TechFile *tech;

    CircuitModel *circuitModel;
    
    int                           NMOS_BIAS;
    int                           PMOS_FIRST;
    int                           NMOS_FIRST;
    int                           MAX_PIN_NUM;
    std::vector<NetNode*>         nets;
    std::vector<std::vector<int>> vcg;
    int                           track;

    std::vector<std::vector<Block>> blocks;
    double                          rowUnit;
    double                          colUnit;

    bool channelRouting ();
    bool gridRouting    ();
    
    void channelCost  ();
    void channelRough ();
    void channelDetail();
    
    void gridCost   ();
    void gridRough  ();
    void gridDetail ();
};

enum BlockValue
{
  SPACE     = -1,
  OBSTACLE  = -2
};

struct Block
{
  int                   value       = SPACE;
  int                   crossNum    = 0;
  int                   detour      = 0;
  NetNode               *connectNet;
  std::vector<NetNode*> crossNet;
};

inline ICRouting::ICRouting( TechFile *techFile )
  : tech( techFile ) {}

inline void ICRouting::setTechFile( TechFile *techFile ) { tech = techFile; }

#endif
