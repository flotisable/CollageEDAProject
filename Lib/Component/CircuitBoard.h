#ifndef CIRCUIT_BOARD_H
#define CIRCUIT_BOARD_H

#include <vector>
#include <string>
using namespace std;

class TechFile;
class Hspice;
class ICPlacement;
class ICRouting;
class Layout;
class CircuitModel;

class CircuitBoard
{
  public:
  
    inline CircuitBoard(  TechFile    *tech   = nullptr ,
                          Hspice      *parser = nullptr ,
                          ICPlacement *placer = nullptr ,
                          ICRouting   *router = nullptr ,
                          Layout      *layout = nullptr );
    ~CircuitBoard();

    inline vector<CircuitModel*>& model();

    inline void setTechFile ( TechFile    *tech   );
    inline void setParser   ( Hspice      *parser );
    inline void setPlacer   ( ICPlacement *placer );
    inline void setRouter   ( ICRouting   *router );
    inline void setLayout   ( Layout      *layout );

    bool process();

    CircuitModel* searchModel( const string &name );

  private:
  
    void mergeModel();
  
    TechFile    *mTech;
    Hspice      *mParser;
    ICPlacement *mPlacer;
    ICRouting   *mRouter;
    Layout      *mLayout;
    
    vector<CircuitModel*> models;
};

inline CircuitBoard::CircuitBoard(  TechFile    *tech   , Hspice    *parser ,
                                    ICPlacement *placer , ICRouting *router ,
                                    Layout      *layout )
  : mTech( tech ) , mParser( parser ) , mPlacer( placer ) , mRouter( router ) ,
    mLayout( layout ) {}

inline vector<CircuitModel*>& CircuitBoard::model() { return models; }

inline void CircuitBoard::setTechFile ( TechFile    *tech   )
{ mTech   = tech;   }
inline void CircuitBoard::setParser   ( Hspice      *parser )
{ mParser = parser; }
inline void CircuitBoard::setPlacer   ( ICPlacement *placer )
{ mPlacer = placer; }
inline void CircuitBoard::setRouter   ( ICRouting   *router )
{ mRouter = router; }
inline void CircuitBoard::setLayout   ( Layout      *layout )
{ mLayout = layout; }

#endif
