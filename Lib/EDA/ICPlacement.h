#ifndef IC_PLACEMENT_H
#define IC_PLACEMENT_H

class CircuitModel;
class TechFile;

class ICPlacement
{
  public:
  
    inline ICPlacement( TechFile *techFile = nullptr );

    inline void setTechFile( TechFile *techFile );

    bool placement( CircuitModel *model );
  
  private:

    TechFile *tech;

    bool mosPlacement     ( CircuitModel *model );
    bool circuitPlacement ( CircuitModel *model );
};

inline ICPlacement::ICPlacement( TechFile *techFile ) : tech( techFile ) {}

inline void ICPlacement::setTechFile( TechFile *techFile ) { tech = techFile; }

#endif
