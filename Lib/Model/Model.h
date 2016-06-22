#ifndef MODEL_H
#define MODEL_H

#include "../Graphic/Rectangle.h"

class Model : public Rectangle
{
  public:

    enum Type
    {
      UNKNOWN = -1,
      MOS,
      SUBCKT,
      TYPE_NUM
    };

    inline Model( Type t = UNKNOWN );

    inline Type type() const;

    inline void setType( Type t );

  private:
  
    Type m_type;
};

// Model inline member function
inline Model::Model( Type t ) : m_type( t ) {}

inline Model::Type Model::type() const { return m_type; }

inline void Model::setType( Type t ) { m_type = t; }
// end Model inline member function

#endif
