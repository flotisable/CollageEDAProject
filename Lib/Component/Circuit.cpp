#include "Circuit.h"

#include "../Component/Mos.h"
#include "../Model/MosModel.h"
#include "../Model/CircuitModel.h"
#include "../Node/NetNode.h"

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

Node* Circuit::searchNode( Node::Type type , const string &name )
{
  if( type == Node::UNKNOWN ) return nullptr;

  for( Node *node : nodes[type] )
     if( node->name() == name ) return node;
  return nullptr;
}

Model* Circuit::searchModel( Model::Type type , Model *model )
{
  if( !model ) return nullptr;

  switch( type )
  {
    case Model::MOS:      return searchMos    ( static_cast<MosModel*>
                                              ( model ) );
    case Model::CIRCUIT:  return searchCircuit( static_cast<CircuitModel*>
                                              ( model ) );
    default:              return nullptr;
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


Model* Circuit::searchMos( MosModel *model )
{
  for( Model *mosModel : models[Model::MOS] )
     if( *static_cast<MosModel*>( mosModel ) == *model ) return mosModel;
  return nullptr;
}

Model* Circuit::searchCircuit( CircuitModel *model )
{
  for( Model *circuitModel : models[Model::CIRCUIT] )
     if( model->name() == static_cast<CircuitModel*>( circuitModel )->name() )
       return circuitModel;
  return nullptr;
}
