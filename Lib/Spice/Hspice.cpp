#include"Hspice.h"

#include <cctype>

#include "../Model/CircuitModel.h"
#include "../Model/MosModel.h"
#include "../Node/NetNode.h"
#include "../Node/MosNode.h"
#include "../Node/CircuitNode.h"
#include "../Component/CircuitBoard.h"

const string  Hspice::SUBCKT_HEAD_KEYWORD = ".SUBCKT";
const string  Hspice::SUBCKT_TAIL_KEYWORD = ".ENDS";

Hspice::Hspice()
{
  id.resize( ID_NUM );
  id[VDD  ] = "VDD";
  id[GND  ] = "GND";
  id[PMOS ] = "PCH";
  id[NMOS ] = "NCH";
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
        CircuitModel *model = circuitBoard->searchModel( word[SUBCKT_NAME] );

        if( !model )
        {
          model = new CircuitModel( word[SUBCKT_NAME] );
          circuitBoard->model().push_back( model );
        }
        setupModel( model );
      }
    }
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
    for( CircuitModel *model : circuitBoard->model() )
       writeCircuitModel( model );
		file.close();
		return true;
	}
	return false;
}


void Hspice::setupModel( Circuit *model )
{
  // io pin
  for( unsigned int i = SUBCKT_NET ; i < word.size() ; i++ )
	{
    NetNode *node = new NetNode( word[i] );

    if      (	word[i] == id[VDD] )  node->setType( Node::VDD  );
    else if ( word[i] == id[GND] )  node->setType( Node::VSS  );
    else                            node->setType( Node::IO   );

    model->io().push_back( node );
	}

  while( getline( file , buffer ) ) // net and cell
  {
    if( buffer.empty()  ) continue;
    getWord();
    if( word[0] == SUBCKT_TAIL_KEYWORD || file.eof() )	return;

    switch( word[0][0] )
    {
      case 'M': setupMos    ( model );  break;
      case 'X': setupSubckt ( model );  break;
      default:                          break;
    }
  }
}

void Hspice::setupMos( Circuit *model )
{
  unsigned int  wordIndex = 0;
  MosNode       *node     = new MosNode( word[M_NAME] );

  model->mosCell().push_back( node );

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

  Model *modelT = model->searchModel( Model::MOS , mosModel );

  if( !modelT )
  {
    modelT = mosModel;
    model->mosModel().push_back( mosModel );
  }
  else
    delete mosModel;

  node->setModel( static_cast<MosModel*>( modelT ) );
}

void Hspice::setupSubckt( Circuit *model )
{
  CircuitNode *node = new CircuitNode( word[X_NAME] );

  model->circuitCell().push_back( node );

  // set net
  for( unsigned int i = X_NET ; i < word.size() - 1 ; i++ )
     setupNode( node , model , word[i] );
     
  CircuitModel *circuitModel = circuitBoard->searchModel( word.back() );

  if( !circuitModel )
  {
    circuitModel = new CircuitModel( word.back() );

    circuitBoard->model().push_back( circuitModel );
  }
  else
    circuitModel->setMainCircuit( false );
  node->setModel( circuitModel );
}

void Hspice::setupNode( Node *node , Circuit *model , const string &netName )
{
  Node *nodeT = model->searchNode( Node::NET , netName );

  if( !nodeT )
    if( !( nodeT = model->searchNode( Node::IO , netName ) ) )
    {
      nodeT = new NetNode( netName );
      model->net().push_back( nodeT );
    }
  node->connect().push_back( nodeT );

  if( nodeT->searchConnectNode( node->name() ) == -1 )
    nodeT->connect().push_back( node );
}

void Hspice::getWord()
{
  word.clear();
  for( unsigned int i = 0 ; i < buffer.size() ; i++ )
  {
     if( buffer[i] == '*' ) return;
     if( !isNullChar( buffer[i] ) )
     {
       unsigned int j;
       
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


void Hspice::writeCircuitModel( CircuitModel *model )
{
  file << "subckt name : " << model->name   ()  << endl;
  file << "height      : " << model->height ()  << endl;
  file << "width       : " << model->width  ()  << endl;
  file << "minHeight   : " << model->minRect().height () << endl;
  file << "minWidth    : " << model->minRect().width  () << endl;
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
