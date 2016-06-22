#ifndef IC_MODEL_H
#define IC_MODEL_H

#include <vector>
#include <string>
using namespace std;

#include "../Node/Node.h"
#include "../Model/Model.h"

class TechFile;
class MosModel;
class SubcktModel;

class ICModel
{
  public:

    ICModel( TechFile *techFile = nullptr );
    ~ICModel();

    inline vector<Node*>& io        ();
    inline vector<Node*>& net       ();
    inline vector<Node*>& mosCell   ();
    inline vector<Node*>& subcktCell();

    inline vector<Model*>&  mosModel   ();
    inline vector<Model*>&  subcktModel();

    inline int  ioNum  () const;
    inline int  netNum () const;
    int         cellNum() const;
    int         nodeNum() const;

    inline bool isMainCircuit() const;

    inline void setTechFile   ( TechFile 	*techFile  	);
    inline void setMainCircuit( bool      mainCircuit	);

    int searchNode  ( Node  ::Type type , const string 	&name  	);
    int searchModel ( Model ::Type type , Model         *model  );

    bool generate ();

  private:

    int searchMos   ( MosModel    *model );
    int searchSubckt( SubcktModel *model );

    TechFile  *tech;
    bool      main;

    vector<vector<Node*>>   nodes;
    vector<vector<Model*>>  models;
};

// ICModel inline member function
inline vector<Node*>& ICModel::io        () { return nodes[Node::IO];     }
inline vector<Node*>& ICModel::net       () { return nodes[Node::NET];    }
inline vector<Node*>& ICModel::mosCell   () { return nodes[Node::MOS];    }
inline vector<Node*>& ICModel::subcktCell() { return nodes[Node::SUBCKT]; }

inline vector<Model*>& ICModel::mosModel    ()
{ return models[Model::MOS];    }
inline vector<Model*>& ICModel::subcktModel ()
{ return models[Model::SUBCKT]; }

inline int  ICModel::ioNum  () const { return nodes[Node::IO] .size(); }
inline int  ICModel::netNum () const { return nodes[Node::NET].size(); }

inline bool ICModel::isMainCircuit() const { return main; }

inline void ICModel::setTechFile   ( TechFile *techFile  )
{ tech = techFile;    }
inline void ICModel::setMainCircuit( bool mainCircuit    )
{ main = mainCircuit; }
// end ICModel inline member function

#endif
