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
	for( vector<Node*> &nodeVector : nodes )
     for( Node *node : nodeVector )
        if( node ) delete node;

	for( vector<Model*> &modelVector : models )
     for( Model *model : modelVector )
        if( model ) delete model;
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

  for( auto &nodeVector : nodes )
     number += nodeVector.size();
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

  for( Model *model : models[Model::MOS] )
  {
     Mos *mos = static_cast<MosModel*>( model );

     mos->setTechFile( tech );
     mos->generate();
  }

  for( Model *model : models[Model::SUBCKT] )
  {
     ICModel *icModel = static_cast<SubcktModel*>( model );

     icModel->setTechFile( tech );
     icModel->generate();
  }
  return true;
}


int ICModel::searchMos( MosModel *model )
{
  for( register unsigned int i = 0 ; i < models[Model::MOS].size() ; i++ )
  {
     MosModel *mosModel = static_cast<MosModel*>( models[Model::MOS][i] );
     if( *mosModel == *model ) return i;
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
