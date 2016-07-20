#ifndef IC_ROUTING_H
#define IC_ROUTING_H

class CircuitModel;
class TechFile;

class ICRouting
{
  public:

    inline ICRouting( CircuitModel  *model    = nullptr ,
                      TechFile      *techFile = nullptr );

    inline CircuitModel* model();

    inline void setModel    ( CircuitModel  *model    );
    inline void setTechFile ( TechFile      *techFile );

    bool routing();

  private:

    TechFile      *tech;
    CircuitModel  *mModel;

    bool channelRouting ( CircuitModel *model );
    bool gridRouting    ( CircuitModel *model );
};

inline ICRouting::ICRouting( CircuitModel *model , TechFile *techFile )
: tech( techFile ) , mModel( model ) {}

inline CircuitModel* ICRouting::model() { return mModel; }

inline void ICRouting::setModel   ( CircuitModel *model   )
{ mModel = model;    }
inline void ICRouting::setTechFile( TechFile    *techFile )
{ tech    = techFile; }

#endif
