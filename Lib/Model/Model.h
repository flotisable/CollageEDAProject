#ifndef MODEL_H
#define MODEL_H

#include "../Graphic/Rectangle.h"

class Model : public Rectangle
{
  public:

    enum Type
    {
      MOS,
      CIRCUIT,
      TYPE_NUM,
      UNKNOWN
    };

    inline Model( Type type = UNKNOWN );

    inline Type type() const;

    inline void setType( Type type );

  private:

    Type mType;
};

// Model inline member function
inline Model::Model( Type type ) : mType( type ) {}

inline Model::Type Model::type() const { return mType; }

inline void Model::setType( Type type ) { mType = type; }
// end Model inline member function

#endif
