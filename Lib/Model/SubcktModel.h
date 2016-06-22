#ifndef SUBCKT_MODEL_H
#define SUBCKT_MODEL_H

#include <string>
using namespace std;

#include "Model.h"

class ICModel;

class SubcktModel : public Model
{
  public:

    inline SubcktModel( ICModel *model = nullptr );

    inline const string&  name  () const;
    inline ICModel*       model () const;

    inline void setName ( const string  &name   );
    inline void setModel( ICModel       *model  );

  private:

    string  m_name;
    ICModel *m_model;
};

// SubcktModel inline member function
inline SubcktModel::SubcktModel( ICModel *model )
: Model( SUBCKT ) , m_model( model ) {}

inline const string&  SubcktModel::name  () const { return m_name;   }
inline ICModel*       SubcktModel::model () const { return m_model;  }

inline void SubcktModel::setName  ( const string &name  ) { m_name  = name;   }
inline void SubcktModel::setModel ( ICModel      *model ) { m_model = model;  }
// SubcktModel inline member function

#endif
