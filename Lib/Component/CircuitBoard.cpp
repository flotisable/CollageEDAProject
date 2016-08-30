#include "CircuitBoard.h"

#include <queue>
#include <algorithm>

#include "../Model/CircuitModel.h"
#include "../Spice/Hspice.h"
#include "../Layout/Layout.h"
#include "../Node/CircuitNode.h"

CircuitBoard::~CircuitBoard()
{
  for( CircuitModel *model : models )
     if( model ) delete model;
}

bool CircuitBoard::process()
{
  mergeModel();

  for( CircuitModel *model : models )
  {
    model->setPlacement ( mPlacer );
    model->setRouting   ( mRouter );
    model->setTechFile  ( mTech   );
    model->layout       ();

    mLayout->setCenter  ( 0 , 0 );
    mLayout->drawCircuit( model );
    mLayout->drawRect   ( Layer::NWELL , *model );
  }
  return true;
}

void CircuitBoard::mergeModel()
{
  int                   mainCircuitNum = 0;
  queue<CircuitModel*>  pipe;

  for( CircuitModel *model : models )
     if( model->isMainCircuit() )
     {
       models[mainCircuitNum] = model;
       mainCircuitNum++;
     }
  models.resize( mainCircuitNum );

  for( CircuitModel *model : models )
  {
     vector<Model*> &circuitModels = model->circuitModel();

     pipe.push( model );

     while( pipe.size() )
     {
       Model *parent = model->searchModel( Model::CIRCUIT , pipe.front() );

       for( Node *node : pipe.front()->circuitCell() )
       {
          CircuitModel *circuit = static_cast<CircuitNode*>( node )->model();

          if( !model->searchModel( Model::CIRCUIT , circuit ) )
          {
            pipe.push( circuit );
            circuitModels.push_back( circuit );
          }
          else if( parent )
          {
            circuitModels.erase( find(  circuitModels.begin() ,
                                        circuitModels.end() , circuit ) );
            pipe.push( circuit );
            circuitModels.push_back( circuit );
          }
       }
       pipe.pop();
     }
     reverse( circuitModels.begin() , circuitModels.end() );
  }
}

CircuitModel* CircuitBoard::searchModel( const string &name )
{
  for( CircuitModel *model : models )
     if( model->name() == name ) return model;
  return nullptr;
}
