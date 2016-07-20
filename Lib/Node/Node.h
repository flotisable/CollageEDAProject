#ifndef NODE_H
#define NODE_H

#include <vector>
#include <string>
using namespace std;

#include "../Graphic/Rectangle.h"

class Node : public Rectangle
{
  public:

    enum Type
    {
      UNKNOWN = -1,
      VDD,
      VSS,
      IO,
      NET,
      MOS,
      CIRCUIT,
      TYPE_NUM
    };

		inline Node( Type t = UNKNOWN );

    static inline bool costCompare( Node* front , Node* back );
    static inline bool costCompare( Node  front , Node  back );

    inline const string&  name    () const;
    inline int            visit   () const;
    inline Type           type    () const;
    inline int            cost    () const;
    inline vector<Node*>& connect ();

    inline void setName ( const string  &s    );
    inline void setVisit( int           i     );
    inline void setType ( Type          t     );
    inline void setCost ( int           cost  );

    int searchConnectNode( const string &name );

  private:

    string  mName;
    int     mVisit;
    Type    mType;
    int     mCost;

    vector<Node*>	mConnect;
};

// Node inline member function
inline Node::Node( Type t ) : mVisit( 0 ) , mType( t ) {}

inline bool Node::costCompare( Node* front , Node* back )
{ return front->cost() > back->cost(); }
inline bool Node::costCompare( Node  front , Node  back )
{ return front.cost() > back.cost(); }

inline const string&  Node::name    () const  { return mName;    }
inline int            Node::visit   () const  { return mVisit;   }
inline Node::Type     Node::type    () const  { return mType;    }
inline int            Node::cost    () const  { return mCost;    }
inline vector<Node*>& Node::connect ()        { return mConnect; }

inline void Node::setName ( const string  &s    ) { mName  = s;    }
inline void Node::setVisit( int           i     ) { mVisit = i;    }
inline void Node::setType ( Type          t     ) { mType  = t;    }
inline void Node::setCost ( int           cost  ) { mCost  = cost; }
// end Node inline member function

#endif
