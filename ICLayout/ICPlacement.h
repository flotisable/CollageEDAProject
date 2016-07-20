#ifndef IC_PLACEMENT_H
#define IC_PLACEMENT_H

class CircuitModel;
class TechFile;

class ICPlacement
{
  public:
  
    inline ICPlacement( CircuitModel  *model    = nullptr ,
                        TechFile      *techFile = nullptr );

    inline CircuitModel* model();

    inline void setModel    ( CircuitModel  *model    );
    inline void setTechFile ( TechFile      *techFile );

    bool placement();
  
  private:

    TechFile      *tech;
    CircuitModel  *mModel;

    bool mosPlacement     ( CircuitModel *model );
    bool circuitPlacement ( CircuitModel *model );
};

inline ICPlacement::ICPlacement( CircuitModel *model , TechFile *techFile )
: tech( techFile ) , mModel( model ) {}

inline CircuitModel* ICPlacement::model() { return mModel; }

inline void ICPlacement::setModel   ( CircuitModel *model )
{ mModel = model; }
inline void ICPlacement::setTechFile( TechFile *techFile  )
{ tech = techFile; }

#endif
