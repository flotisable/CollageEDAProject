#ifndef MOS_NODE_H
#define MOS_NODE_H

#include <ostream>
#include <string>
using namespace std;

#include "Node.h"

class MosModel;

class MosNode : public Node
{
  public:

    enum MosPin
    {
      D,
      G,
      S,
      B,
      PIN_NUM
    };

    inline MosNode( MosModel *model = nullptr );
    inline MosNode( const string &name , MosModel *model = nullptr );

    inline MosModel* model() const;

    inline void setModel( MosModel *model );

  private:

    MosModel *mos;
};

// MosNode non-member function
ostream& operator<<( ostream &out , MosNode &node );
// end MosNode non-member function

// MosNode inline member function
inline MosNode::MosNode( MosModel *model ) : Node( MOS ) , mos( model ) {}
inline MosNode::MosNode( const string &name , MosModel *model )
  : Node( name , MOS ) , mos( model ) {}

inline MosModel* MosNode::model() const { return mos; }

inline void MosNode::setModel( MosModel *model ) { mos = model; }
// end MosNode inline member function

#endif
