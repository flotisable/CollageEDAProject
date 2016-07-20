#include "Circuit.h"

#include "../Component/Mos.h"
#include "../Model/MosModel.h"
#include "../Model/CircuitModel.h"

Circuit::Circuit( TechFile *techFile ) : tech( techFile ) , main( true )
{
  nodes .resize( Node ::TYPE_NUM );
  models.resize( Model::TYPE_NUM );
}

Circuit::~Circuit()
{
	for( vector<Node*> &nodeVector : nodes )
     for( Node *node : nodeVector )
        if( node ) delete node;

	for( vector<Model*> &modelVector : models )
     for( Model *model : modelVector )
        if( model ) delete model;
}

int Circuit::cellNum() const
{
  int number = 0;

  for( register unsigned int i = Node::NET + 1 ; i < nodes.size() ; i++ )
     number += nodes[i].size();
  return number;
}

int Circuit::nodeNum() const
{
  int number = 0;

  for( auto &nodeVector : nodes ) number += nodeVector.size();
  return number;
}

int Circuit::searchNode( Node::Type type , const string &name )
{
  if( type == Node::UNKNOWN ) return -1;

  for( register unsigned int i = 0 ; i < nodes[type].size() ; i++ )
     if( nodes[type][i]->name() == name ) return i;
  return -1;
}

int Circuit::searchModel( Model::Type type , Model *model )
{
  switch( type )
  {
    case Model::MOS:      return searchMos    ( static_cast<MosModel*>
                                              ( model ) );
    case Model::CIRCUIT:  return searchCircuit( static_cast<CircuitModel*>
                                              ( model ) );
    default:              return -1;
  }
}

bool Circuit::generate()
{
  if( !tech ) return false;

  for( Model *model : models[Model::MOS] )
  {
     MosModel *mos = static_cast<MosModel*>( model );

     mos->setTechFile( tech );
     mos->generate();
  }

  for( Model *model : models[Model::CIRCUIT] )
  {
     CircuitModel *circuit = static_cast<CircuitModel*>( model );

     circuit->setTechFile( tech );
     circuit->generate();
  }
  return true;
}


int Circuit::searchMos( MosModel *model )
{
  for( register unsigned int i = 0 ; i < models[Model::MOS].size() ; i++ )
  {
     MosModel *mos = static_cast<MosModel*>( models[Model::MOS][i] );

     if( *mos == *model ) return i;
  }
  return -1;
}

int Circuit::searchCircuit( CircuitModel *model )
{
  for( register unsigned int i = 0 ; i < models[Model::CIRCUIT].size() ; i++ )
  {
     CircuitModel *circuit =  static_cast<CircuitModel*>
                              ( models[Model::CIRCUIT][i] );

     if( model->name() == circuit->name() ) return i;
  }
  return -1;
}
