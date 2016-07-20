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
      CIRCUIT,
      TYPE_NUM
    };

    inline Model( Type t = UNKNOWN );

    inline Type type() const;

    inline void setType( Type t );

  private:
  
    Type mType;
};

// Model inline member function
inline Model::Model( Type t ) : mType( t ) {}

inline Model::Type Model::type() const { return mType; }

inline void Model::setType( Type t ) { mType = t; }
// end Model inline member function

#endif
