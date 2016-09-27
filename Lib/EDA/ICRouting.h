#ifndef IC_ROUTING_H
#define IC_ROUTING_H

#include <vector>
#include <forward_list>
#include <list>
#include <fstream>
#include <queue>

#include "../Graphic/Point.h"

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

    std::vector<NetNode*> nets;
    
    std::fstream debug;
    
    // mos routing variables
    const int PMOS_BIAS = 0;
    int       NMOS_BIAS;
    int       PMOS_FIRST;
    int       NMOS_FIRST;
    int       MAX_PIN_NUM;
    int       track;

    std::vector<std::vector<int>> vcg;
    // end mos routing variables

    // grid routing variables
    double rowUnit;
    double colUnit;

    std::vector<std::vector<Block>>       blocks;
    std::vector<std::forward_list<Point>> ioPos;
    // end grid routing variables

    bool channelRouting ();
    bool gridRouting    ();
    
    void channelCost  ();
    void channelRough ();
    void channelDetail();
    
    void gridCost   ();
    void gridRough  ();
    void gridDetail ();
    
    void findMinimumChannel ();
    void setupObstacle      ();
    
    void setupIOPin         ( NetNode *node , int netIndex , std::forward_list<Point> &ios );
    void hadlocksMazeRouting( std::forward_list<Point> &ios , int netIndex , NetNode *node );
    void leeAlgorithm       ( Point &source , NetNode *node , int netIndex );
    void setupBlockCrossNum ( NetNode *node );
    
    void setupCrossNet( const Point &acceptCrossNum , std::queue<Point> &waitedBlocks );
    void moveNets     ( const Point &acceptCrossNum , std::queue<Point> &waitedBlocks );
    void setupNets    ( const double xUnit , const double yUnit );
};

struct Block
{
  enum Value
  {
    SPACE     = -1,
    OBSTACLE  = -2
  };

  int     value       = SPACE;
  int     detour;
  int     visit       = -1;
  NetNode *connectNet = nullptr;

  std::list<NetNode*>   crossNetX;
  std::list<NetNode*>   crossNetY;
  std::vector<NetNode*> channelX;
  std::vector<NetNode*> channelY;
};

inline ICRouting::ICRouting( TechFile *techFile )
  : tech( techFile ) , circuitModel( nullptr ) {}

inline void ICRouting::setTechFile( TechFile *techFile ) { tech = techFile; }

#endif
