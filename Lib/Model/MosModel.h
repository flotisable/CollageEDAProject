#ifndef MOS_MODEL_H
#define MOS_MODEL_H

#include "Model.h"
#include "../Component/Mos.h"

class MosModel : public Model
{
  public:
  
    inline MosModel( Mos *model = nullptr );

    inline Mos* model();

    inline void setModel( Mos *model );

  private:

    Mos *m_model;
};

// MosModel inline member function
inline MosModel::MosModel( Mos *model ) : Model( MOS ) , m_model( model ) {}

inline Mos* MosModel::model() { return m_model; }

inline void MosModel::setModel( Mos *model ) { m_model = model; }
// end MosModel inline member function

#endif
