#ifndef IC_ROUTING_H
#define IC_ROUTING_H

class SubcktModel;
class TechFile;

class ICRouting
{
  public:

    inline ICRouting( SubcktModel *model    = nullptr ,
                      TechFile    *techFile = nullptr );

    inline SubcktModel* model();

    inline void setModel    ( SubcktModel *model    );
    inline void setTechFile ( TechFile    *techFile );

    bool routing();

  private:

    TechFile    *tech;
    SubcktModel *m_model;

    bool channelRouting ( SubcktModel *model );
    bool gridRouting    ( SubcktModel *model );
};

inline ICRouting::ICRouting( SubcktModel *model , TechFile *techFile )
: tech( techFile ) , m_model( model ) {}

inline SubcktModel* ICRouting::model() { return m_model; }

inline void ICRouting::setModel   ( SubcktModel *model    )
{ m_model = model;    }
inline void ICRouting::setTechFile( TechFile    *techFile )
{ tech    = techFile; }

#endif
