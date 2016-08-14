#ifndef IC_ROUTING_H
#define IC_ROUTING_H

#include <vector>

class CircuitModel;
class TechFile;
class NetNode;

class ICRouting
{
  public:

    inline ICRouting( TechFile *techFile = nullptr );

    inline void setTechFile( TechFile *techFile );

    bool routing( CircuitModel *model );

  private:

    TechFile *tech;

    CircuitModel *circuitModel;
    
    int                           nmosBias;
    int                           pmosFirst;
    int                           nmosFirst;
    std::vector<NetNode*>         nets;
    const int                     VCG;
    std::vector<std::vector<int>> vcg;
    int                           track;

    bool channelRouting ( CircuitModel *model );
    bool gridRouting    ( CircuitModel *model );
    
    void channelCost  ();
    void channelRough ();
    void channelDetail();
};

inline ICRouting::ICRouting( TechFile *techFile )
  : tech( techFile ) , VCG( 2 ) {}

inline void ICRouting::setTechFile( TechFile *techFile ) { tech = techFile; }

#endif
