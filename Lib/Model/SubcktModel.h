#ifndef SUBCKT_MODEL_H
#define SUBCKT_MODEL_H

#include <string>
using namespace std;

#include "Model.h"
#include "ICModel.h"

class SubcktModel : public Model , public ICModel
{
  public:

    inline SubcktModel();

    inline const string& name() const;

    inline void setName( const string &name );

  private:

    string  m_name;
};

// SubcktModel inline member function
inline SubcktModel::SubcktModel() : Model( SUBCKT ) {}

inline const string& SubcktModel::name() const { return m_name;   }

inline void SubcktModel::setName( const string &name ) { m_name = name; }
// SubcktModel inline member function

#endif
