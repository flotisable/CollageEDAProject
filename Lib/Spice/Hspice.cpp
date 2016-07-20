#include"Hspice.h"

#include <ctype.h>
#include <queue>

#include "../Model/CircuitModel.h"
#include "../Model/MosModel.h"
#include "../Node/NetNode.h"
#include "../Node/MosNode.h"
#include "../Node/CircuitNode.h"
#include "../Component/Circuit.h"
#include "../Component/Mos.h"

const string  Hspice::SUBCKT_HEAD_KEYWORD = ".SUBCKT";
const string  Hspice::SUBCKT_TAIL_KEYWORD = ".ENDS";
const int     Hspice::MAIN                = 0;

Hspice::Hspice( TechFile *techFile ) : tech( techFile )
{
  id.resize( ID_NUM );
  id[VDD  ] = "VDD";
  id[GND  ] = "GND";
  id[PMOS ] = "PCH";
  id[NMOS ] = "NCH";
}

Hspice::~Hspice()
{
  for( CircuitModel *model : models )
     if( model ) delete model;
}

bool Hspice::read( const char *fileName )
{
  if( file.is_open() ) file.close();
	file.open( fileName , ios::in );
	
	if( file.is_open() )
	{
    getline( file , buffer ); // comment

    // read data
    while( getline( file , buffer ) )
    {
      if( buffer.empty() || buffer[0] == '*' ) continue;
      getWord();

			if( word[SUBCKT_ID] == SUBCKT_HEAD_KEYWORD ) // read subckt
      {
        int index = searchModel( word[SUBCKT_NAME] );
        
        if( index == -1 )
        {
          CircuitModel *model = new CircuitModel;
        
          model->setName    ( word[SUBCKT_NAME] );
          model->setTechFile( tech );

          models.push_back( model );
          index = models.size() - 1;
        }
        setupModel( index );
      }
    }
    mergeModel();

		file.close();
		return true;
	}
  return false;
}

bool Hspice::write( const char *fileName )
{
  if( file.is_open() ) file.close();
	file.open( fileName , ios::out );
	
	if( file.is_open() )
	{
    for( CircuitModel *model : models ) writeCircuitModel( model );
		file.close();
		return true;
	}
	return false;
}

void Hspice::mergeModel()
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
       Circuit  *temp       = pipe.front();
       int      parentIndex = model->searchModel( Model::CIRCUIT ,
                                                  pipe.front() );

       for( Node *node : temp->circuitCell() )
       {
          CircuitModel  *circuit  = static_cast<CircuitNode*>( node )
                                    ->model();
          int           index     = model->searchModel( Model::CIRCUIT ,
                                                        circuit );

          if( index == -1 )
          {
            pipe.push( circuit );
            circuitModels.push_back( circuit );
          }
          else
            if( index < parentIndex )
              swap( circuitModels[index] , circuitModels[parentIndex] );
       }
       pipe.pop();
     }
  }
}


void Hspice::setupModel( int index )
{
  Circuit *model = models[index];

  // io pin
  for( register unsigned int i = SUBCKT_NET ; i < word.size() ; i++ )
	{
    NetNode *node = new NetNode;

    node->setName( word[i] );

    if      (	word[i] == id[VDD] )  node->setType( Node::VDD  );
    else if ( word[i] == id[GND] )  node->setType( Node::VSS  );
    else                            node->setType( Node::IO   );

    model->io().push_back( node );
	}

  while( getline( file , buffer ) ) // net and cell
  {
    if( buffer.empty() ) continue;
    getWord();
    if( word[0] == SUBCKT_TAIL_KEYWORD || file.eof() )	return;

    switch( word[0][0] )
    {
      case 'M': setupMos    ( model ); break;
      case 'X': setupSubckt ( model ); break;
    }
  }
}

void Hspice::setupMos( Circuit *model )
{
  unsigned int  wordIndex = 0;
  MosNode       *node     = new MosNode;

  model->mosCell().push_back( node );
  node->setName( word[M_NAME] );

  // set net
  for( int i = D ; i < D + MosNode::PIN_NUM ; i++ )
     setupNode( node , model , word[i] );
  wordIndex += MosNode::PIN_NUM;
  // end set net

  // set mos
  MosModel *mosModel = new MosModel;

  wordIndex++;
  if      (	word[TYPE] == id[PMOS] ) mosModel->Mos::setType( Mos::PMOS );
  else if (	word[TYPE] == id[NMOS] ) mosModel->Mos::setType( Mos::NMOS );

  wordIndex++;
  if( wordIndex < word.size() )
    mosModel->setW( stod( word[W].substr( 2 , word[W].size() - 3 ) ) );
  else
    mosModel->setW( 0 );

  wordIndex++;
  if( wordIndex < word.size() )
    mosModel->setL( stod( word[L].substr( 2 , word[L].size() - 3 ) ) );
  else
    mosModel->setL( 0 );

  wordIndex++;
  if( wordIndex < word.size() )
    mosModel->setM( stod( word[M].substr( 2 , word[M].size() - 3 ) ) );
  else
    mosModel->setM( 1 );
  // end set mos

  int index = model->searchModel( Model::MOS , mosModel );

  if( index == -1 )
  {
    model->mosModel().push_back( mosModel );
    index = model->mosModel().size() - 1;
  }
  else
    delete mosModel;

  node->setModel( static_cast<MosModel*>( model->mosModel()[index] ) );
}

void Hspice::setupSubckt( Circuit *model )
{
  CircuitNode *node = new CircuitNode;

  model->circuitCell().push_back( node );
  node->setName( word[X_NAME] );

  // ste net
  for( register unsigned int i = X_NET ; i < word.size() - 1 ; i++ )
     setupNode( node , model , word[i] );
     
  int index = searchModel( word.back() );

  if( index == -1 )
  {
    CircuitModel *circuitModel = new CircuitModel;
    
    circuitModel->setTechFile( tech );
  
    models.push_back( circuitModel );
    models.back()->setName( word.back() );

    index = models.size() - 1;
  }
  else
    models[index]->setMainCircuit( false );
  node->setModel( models[index] );
}

void Hspice::setupNode( Node *node , Circuit *model , const string &netName )
{
  int index = model->searchNode( Node::NET , netName );

  if( index == -1 )
  {
    index = model->searchNode( Node::IO , netName );

    if( index == -1 )
    {
      model->net().push_back( new NetNode );
      model->net().back()->setName( netName );
      index = model->net().size() - 1;
    }
    else
    {
      node->connect().push_back( model->io()[index] );
      if( model->io()[index]->searchConnectNode( node->name() ) == -1 )
        model->io()[index]->connect().push_back( node );
      return;
    }
  }
  node->connect().push_back( model->net()[index] );

  if( model->net()[index]->searchConnectNode( node->name() ) == -1 )
    model->net()[index]->connect().push_back( node );
}

void Hspice::getWord()
{
  word.clear();
  for( register unsigned int i = 0 ; i < buffer.size() ; i++ )
  {
     if( buffer[i] == '*' ) return;
     if( !isNullChar( buffer[i] ) )
     {
       register unsigned int j;
       
       for( j = i ; j < buffer.size() ; j++ )
       {
          if( isNullChar( buffer[j] ) ) break;
          buffer[j] = toupper( buffer[j] );
       }
       word.push_back( buffer.substr( i , j - i ) );
       i = j;
     }
  }
}

int Hspice::searchModel( const string &name )
{
  for( register unsigned int i = 0 ; i < models.size() ; i++ )
     if( models[i]->name() == name ) return i;
  return -1;
}

void Hspice::writeCircuitModel( CircuitModel *model )
{
  file << "subckt name : " << model->name   ()  << endl;
  file << "height      : " << model->height ()  << endl;
  file << "width       : " << model->width  ()  << endl;
  file << "area        : " << model->area   ()  << endl;
  file << "nodenum     : " << model->nodeNum()  << endl;
  file << "netnum      : " << model->netNum ()  << endl;
  file << "ionum       : " << model->ioNum  ()  << endl;

  for( Node *node : model->io() )
     file << *static_cast<NetNode*>( node ) << endl;

  for( Node *node : model->net() )
		 file << *static_cast<NetNode*>( node ) << endl;

	for( Node *node : model->mosCell() )
		 file << *static_cast<MosNode*>( node ) << endl;

	for( Node *node : model->circuitCell() )
     file << *static_cast<CircuitNode*>( node ) << endl;

  file << endl;

  for( Model *circuitModel : model->circuitModel() )
     writeCircuitModel( static_cast<CircuitModel*>( circuitModel ) );
  file << endl;
}
