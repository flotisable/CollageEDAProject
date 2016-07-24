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
      VDD,
      VSS,
      IO,
      NET,
      MOS,
      CIRCUIT,
      TYPE_NUM,
      UNKNOWN
    };

		inline Node( Type type = UNKNOWN );
		inline Node( const string &name , Type type = UNKNOWN );

    static inline bool costCompare( Node* front , Node* back );
    static inline bool costCompare( Node  front , Node  back );

    inline const string&  name    () const;
    inline Type           type    () const;
    inline int            visit   () const;
    inline int            cost    () const;
    inline vector<Node*>& connect ();

    inline void setName ( const string  &name );
    inline void setType ( Type          type  );
    inline void setVisit( int           visit );
    inline void setCost ( int           cost  );

    int searchConnectNode( const string &name );

  private:

    string  mName;
    Type    mType;
    int     mVisit;
    int     mCost;

    vector<Node*>	mConnect;
};

// Node inline member function
inline Node::Node( Type type ) : mType( type ) , mVisit( 0 ) {}
inline Node::Node( const string &name , Type type )
  : mName( name ) , mType( type ) , mVisit( 0 ) {}

inline bool Node::costCompare( Node* front , Node* back )
{ return front->cost() > back->cost(); }
inline bool Node::costCompare( Node  front , Node  back )
{ return front.cost() > back.cost(); }

inline const string&  Node::name    () const  { return mName;    }
inline Node::Type     Node::type    () const  { return mType;    }
inline int            Node::visit   () const  { return mVisit;   }
inline int            Node::cost    () const  { return mCost;    }
inline vector<Node*>& Node::connect ()        { return mConnect; }

inline void Node::setName ( const string  &name ) { mName  = name;  }
inline void Node::setType ( Type          type  ) { mType  = type;  }
inline void Node::setVisit( int           visit ) { mVisit = visit; }
inline void Node::setCost ( int           cost  ) { mCost  = cost;  }
// end Node inline member function

#endif
