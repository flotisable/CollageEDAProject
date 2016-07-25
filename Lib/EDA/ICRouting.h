#ifndef IC_ROUTING_H
#define IC_ROUTING_H

class CircuitModel;
class TechFile;

class ICRouting
{
  public:

    inline ICRouting( TechFile *techFile = nullptr );

    inline void setTechFile( TechFile *techFile );

    bool routing( CircuitModel *model );

  private:

    TechFile *tech;

    bool channelRouting ( CircuitModel *model );
    bool gridRouting    ( CircuitModel *model );
};

inline ICRouting::ICRouting( TechFile *techFile ) : tech( techFile ) {}

inline void ICRouting::setTechFile( TechFile *techFile ) { tech = techFile; }

#endif
