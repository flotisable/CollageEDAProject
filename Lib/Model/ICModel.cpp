#include "ICModel.h"

#include "../Component/Mos.h"
#include "MosModel.h"
#include "SubcktModel.h"

ICModel::ICModel( TechFile *techFile ) : tech( techFile ) , main( true )
{
  nodes .resize( Node ::TYPE_NUM );
  models.resize( Model::TYPE_NUM );
}

ICModel::~ICModel()
{
	for( unsigned int i = 0 ; i < nodes.size() ; i++ )
     for( unsigned int j = 0 ; j < nodes[i].size() ; j++ )
        if( nodes[i][j] ) delete nodes[i][j];

	for( unsigned int i = 0 ; i < models.size() ; i++ )
     for( unsigned int j = 0 ; j < models[i].size() ; j++ )
        if( models[i][j] )
        {
          switch( models[i][j]->type() )
          {
            case Model::MOS:
            {
              MosModel *model = static_cast<MosModel*>( models[i][j] );
              if( model->model() ) delete model->model();
              break;
            }
            case Model::SUBCKT:
            {
              SubcktModel *model = static_cast<SubcktModel*>( models[i][j] );
              if( model->model() ) delete model->model();
              break;
            }
            default: break;
          }
          delete models[i][j];
        }
}

int ICModel::cellNum() const
{
  int number = 0;

  for( register unsigned int i = Node::NET + 1 ; i < nodes.size() ; i++ )
     number += nodes[i].size();
  return number;
}

int ICModel::nodeNum() const
{
  int number = 0;

  for( register unsigned int i = 0 ; i < nodes.size() ; i++ )
     number += nodes[i].size();
  return number;
}

int ICModel::searchNode( Node::Type type , const string &name )
{
  if( type == Node::UNKNOWN ) return -1;

  for( register unsigned int i = 0 ; i < nodes[type].size() ; i++ )
     if( nodes[type][i]->name() == name ) return i;
  return -1;
}

int ICModel::searchModel( Model::Type type , Model *model )
{
  switch( type )
  {
    case Model::MOS:    return searchMos    ( static_cast<MosModel*>
                                              ( model ) );
    case Model::SUBCKT: return searchSubckt ( static_cast<SubcktModel*>
                                              ( model ) );
    default:            return -1;
  }
}

bool ICModel::generate()
{
  if( !tech ) return false;

  for( unsigned int i = 0 ; i < models[Model::MOS].size() ; i++ )
  {
     Mos *mos = static_cast<MosModel*>( models[Model::MOS][i] )->model();

     mos->setTechFile( tech );
     mos->generate();
  }

  for( unsigned int i = 0 ; i < models[Model::SUBCKT].size() ; i++ )
  {
     ICModel *model = static_cast<SubcktModel*>( models[Model::SUBCKT][i] )
                      ->model();

     model->setTechFile( tech );
     model->generate();
  }
  return true;
}


int ICModel::searchMos( MosModel *model )
{
  for( register unsigned int i = 0 ; i < models[Model::MOS].size() ; i++ )
  {
     MosModel *mosModel = static_cast<MosModel*>( models[Model::MOS][i] );
     if( *mosModel->model() == *model->model() ) return i;
  }
  return -1;
}

int ICModel::searchSubckt( SubcktModel *model )
{
  for( register unsigned int i = 0 ; i < models[Model::SUBCKT].size() ; i++ )
  {
     SubcktModel *subcktModel = static_cast<SubcktModel*>
                                ( models[Model::SUBCKT][i] );
     if( model->name() == subcktModel->name() ) return i;
  }
  return -1;
}
