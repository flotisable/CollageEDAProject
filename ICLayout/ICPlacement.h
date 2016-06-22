#ifndef IC_PLACEMENT_H
#define IC_PLACEMENT_H

class SubcktModel;
class TechFile;

class ICPlacement
{
  public:
  
    inline ICPlacement( SubcktModel *model    = nullptr ,
                        TechFile    *techFile = nullptr );

    inline SubcktModel* model();

    inline void setModel    ( SubcktModel *model    );
    inline void setTechFile ( TechFile    *techFile );

    bool placement();
  
  private:

    TechFile    *tech;
    SubcktModel *m_model;

    bool mosPlacement   ( SubcktModel *model );
    bool subcktPlacement( SubcktModel *model );
};

inline ICPlacement::ICPlacement( SubcktModel *model , TechFile *techFile )
: tech( techFile ) , m_model( model ) {}

inline SubcktModel* ICPlacement::model() { return m_model; }

inline void ICPlacement::setModel( SubcktModel *model )
{ m_model = model; }
inline void ICPlacement::setTechFile( TechFile *techFile )
{ tech = techFile; }

#endif
