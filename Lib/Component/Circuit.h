#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <vector>
#include <string>
using namespace std;

#include "../Node/Node.h"
#include "../Model/Model.h"

class TechFile;
class MosModel;
class CircuitModel;

class Circuit
{
  public:

    inline Circuit( TechFile *techFile = nullptr );
    ~Circuit();

    inline vector<Node*>& io          ();
    inline vector<Node*>& net         ();
    inline vector<Node*>& mosCell     ();
    inline vector<Node*>& circuitCell ();

    inline vector<Model*>&  mosModel    ();
    inline vector<Model*>&  circuitModel();

    inline int  ioNum  () const;
    inline int  netNum () const;
    int         cellNum() const;
    int         nodeNum() const;

    inline bool isMainCircuit() const;

    inline void setTechFile   ( TechFile  *techFile  	);
    inline void setMainCircuit( bool      mainCircuit	);

    Node*   searchNode  ( Node  ::Type type , const string 	&name  	);
    Model*  searchModel ( Model ::Type type , Model         *model  );

    bool generate();

  protected:

    TechFile  *tech;

  private:

    Model* searchMos    ( MosModel      *model  );
    Model* searchCircuit( CircuitModel  *model  );

    bool main;

    vector<vector<Node*>>   nodes;
    vector<vector<Model*>>  models;
};

// ICModel inline member function
inline Circuit::Circuit(  TechFile  *techFile )
  : tech( techFile ) , main( true )
{
  nodes .resize( Node ::TYPE_NUM );
  models.resize( Model::TYPE_NUM );
}

inline vector<Node*>& Circuit::io         () { return nodes[Node::IO];      }
inline vector<Node*>& Circuit::net        () { return nodes[Node::NET];     }
inline vector<Node*>& Circuit::mosCell    () { return nodes[Node::MOS];     }
inline vector<Node*>& Circuit::circuitCell() { return nodes[Node::CIRCUIT]; }

inline vector<Model*>& Circuit::mosModel    ()
{ return models[Model::MOS];    }
inline vector<Model*>& Circuit::circuitModel ()
{ return models[Model::CIRCUIT]; }

inline int Circuit::ioNum  () const { return nodes[Node::IO] .size(); }
inline int Circuit::netNum () const { return nodes[Node::NET].size(); }

inline bool Circuit::isMainCircuit() const { return main; }

inline void Circuit::setTechFile   ( TechFile     *techFile   )
{ tech = techFile;    }
inline void Circuit::setMainCircuit( bool         mainCircuit )
{ main = mainCircuit; }
// end ICModel inline member function

#endif
