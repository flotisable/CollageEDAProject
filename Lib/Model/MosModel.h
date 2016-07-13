#ifndef MOS_MODEL_H
#define MOS_MODEL_H

#include "Model.h"
#include "../Component/Mos.h"

class MosModel : public Model , public Mos
{
  public:
  
    inline MosModel();
};

// MosModel inline member function
inline MosModel::MosModel() : Model( MOS ) {}
// end MosModel inline member function

#endif
