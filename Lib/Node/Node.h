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
      SUBCKT,
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

    string  m_name;
    int     m_visit;
    Type    m_type;
    int     m_cost;

    vector<Node*>	m_connect;
};

// Node inline member function
inline Node::Node( Type t ) : m_visit( 0 ) , m_type( t ) {}

inline bool Node::costCompare( Node* front , Node* back )
{ return front->cost() > back->cost(); }
inline bool Node::costCompare( Node  front , Node  back )
{ return front.cost() > back.cost(); }

inline const string&  Node::name    () const  { return m_name;    }
inline int            Node::visit   () const  { return m_visit;   }
inline Node::Type     Node::type    () const  { return m_type;    }
inline int            Node::cost    () const  { return m_cost;    }
inline vector<Node*>& Node::connect ()        { return m_connect; }

inline void Node::setName ( const string  &s    ) { m_name  = s;    }
inline void Node::setVisit( int           i     ) { m_visit = i;    }
inline void Node::setType ( Type          t     ) { m_type  = t;    }
inline void Node::setCost ( int           cost  ) { m_cost  = cost; }
// end Node inline member function

#endif
