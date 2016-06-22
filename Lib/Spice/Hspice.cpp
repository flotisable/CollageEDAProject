#include"Hspice.h"

#include <ctype.h>
#include <queue>

#include "../Model/SubcktModel.h"
#include "../Model/MosModel.h"
#include "../Model/ICModel.h"
#include "../Node/NetNode.h"
#include "../Node/MosNode.h"
#include "../Node/SubcktNode.h"
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
  for( register int i = 0 ; i < models.size() ; i++ )
     if( models[i] ) delete models[i];
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
          SubcktModel *model = new SubcktModel;
        
          model->setName  ( word[SUBCKT_NAME] );
          model->setModel ( new ICModel( tech ) );

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
    for( int i = 0 ; i < models.size() ; i++ ) writeSubcktModel( models[i] );
		file.close();
		return true;
	}
	return false;
}

void Hspice::mergeModel()
{
  int                 mainCircuitNum = 0;
  queue<SubcktModel*> pipe;

  for( register int i = 0 ; i < models.size() ; i++ )
     if( models[i]->model()->isMainCircuit() )
     {
       models[mainCircuitNum] = models[i];
       mainCircuitNum++;
     }
  models.resize( mainCircuitNum );

  for( int i = 0 ; i < models.size() ; i++ )
  {
     ICModel        *model        = models[i]->model();
     vector<Model*> &subcktModels = model->subcktModel();

     pipe.push( models[i] );

     while( pipe.size() )
     {
       ICModel  *temp       = pipe.front()->model();
       int      parentIndex = model->searchModel( Model::SUBCKT ,
                                                  pipe.front() );

       for( int i = 0 ; i < temp->subcktCell().size() ; i++ )
       {
          SubcktModel *subckt = static_cast<SubcktNode*>
                                ( temp->subcktCell()[i] )->model();
          int         index   = model->searchModel( Model::SUBCKT , subckt );

          if( index == -1 )
          {
            pipe.push( subckt );
            subcktModels.push_back( subckt );
          }
          else
            if( index < parentIndex )
              swap( subcktModels[index] , subcktModels[parentIndex] );
       }
       pipe.pop();
     }
  }
}


void Hspice::setupModel( int index )
{
  ICModel *model = models[index]->model();

  for( register int i = SUBCKT_NET ; i < word.size() ; i++ ) // io pin
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

void Hspice::setupMos( ICModel *model )
{
  int     wordIndex = 0;
  MosNode *node     = new MosNode;

  model->mosCell().push_back( node );
  node->setName( word[M_NAME] );

  // set net
  for( int i = D ; i < D + MosNode::PIN_NUM ; i++ )
     setupNode( node , model , word[i] );
  wordIndex += MosNode::PIN_NUM;
  // end set net

  // set mos
  Mos *mos = new Mos;

  wordIndex++;
  if      (	word[TYPE] == id[PMOS] ) mos->setType( Mos::PMOS );
  else if (	word[TYPE] == id[NMOS] ) mos->setType( Mos::NMOS );

  wordIndex++;
  if( wordIndex < word.size() )
    mos->setW( stod( word[W].substr( 2 , word[W].size() - 3 ) ) );
  else
    mos->setW( 0 );

  wordIndex++;
  if( wordIndex < word.size() )
    mos->setL( stod( word[L].substr( 2 , word[L].size() - 3 ) ) );
  else
    mos->setL( 0 );

  wordIndex++;
  if( wordIndex < word.size() )
    mos->setM( stod( word[M].substr( 2 , word[M].size() - 3 ) ) );
  else
    mos->setM( 1 );
  // end set mos

  MosModel *mosModel = new MosModel( mos );

  int index = model->searchModel( Model::MOS , mosModel );

  if( index == -1 )
  {
    model->mosModel().push_back( mosModel );
    index = model->mosModel().size() - 1;
  }
  else
  {
    delete mos;
    delete mosModel;
  }

  node->setModel( static_cast<MosModel*>( model->mosModel()[index] ) );
}

void Hspice::setupSubckt( ICModel *model )
{
  SubcktNode *node = new SubcktNode;

  model->subcktCell().push_back( node );
  node->setName( word[X_NAME] );

  // ste net
  for( register int i = X_NET ; i < word.size() - 1 ; i++ )
     setupNode( node , model , word[i] );
     
  int index = searchModel( word.back() );

  if( index == -1 )
  {
    models.push_back( new SubcktModel( new ICModel( tech ) ) );
    models.back()->setName( word.back() );

    index = models.size() - 1;
  }
  else
    models[index]->model()->setMainCircuit( false );
  node->setModel( models[index] );
}

void Hspice::setupNode( Node *node , ICModel *model , const string &netName )
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
  for( register int i = 0 ; i < buffer.size() ; i++ )
  {
     if( buffer[i] == '*' ) return;
     if( !isNullChar( buffer[i] ) )
     {
       register int j;
       
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
  for( register int i = 0 ; i < models.size() ; i++ )
     if( models[i]->name() == name ) return i;
  return -1;
}

void Hspice::writeSubcktModel( SubcktModel *model )
{
  vector<Node*>   &io           = model->model()->io();
  vector<Node*>   &net          = model->model()->net();
  vector<Node*>   &mosCell      = model->model()->mosCell();
  vector<Node*>   &subcktCell   = model->model()->subcktCell();
  vector<Model*>  &subcktModel  = model->model()->subcktModel();

  file << "subckt name : " << model->name()               << endl;
  file << "height      : " << model->height()             << endl;
  file << "width       : " << model->width()              << endl;
  file << "area        : " << model->area()               << endl;
  file << "nodenum     : " << model->model()->nodeNum ()  << endl;
  file << "netnum      : " << model->model()->netNum  ()  << endl;
  file << "ionum       : " << model->model()->ioNum   ()  << endl;

  for( int i = 0 ; i < io.size() ; i++ )
	{
	   file << io[i]->name() << "\t";
		 file << io[i]->type() << "\t";

		 for( register int j = 0 ; j < io[i]->connect().size() ; j++ )
				file << io[i]->connect()[j]->name() << "\t";

     NetNode *node = static_cast<NetNode*>( io[i] );

     for( register int j = 0 ; j < node->nets().size() ; j++ )
        file << node->nets()[j]/*.center()*/;

		 file << endl;
	}

  for( int i = 0 ; i < net.size() ; i++ )
	{
	   file << net[i]->name() << "\t";
	   file << net[i]->type() << "\t";

		 for( register int j = 0 ; j < net[i]->connect().size() ; j++ )
		    file << net[i]->connect()[j]->name() << "\t";
     
     NetNode* node = static_cast<NetNode*>( net[i] );

     for( register int j = 0 ; j < node->nets().size() ; j++ )
        file << node->nets()[j]/*.center()*/;

		 file << endl;
	}

	for( int i = 0 ; i < mosCell.size() ; i++ )
	{
     Mos* mos = static_cast<MosNode*>( mosCell[i] )->model()->model();

     file << mosCell[i]->name()   << "\t";
     file << mosCell[i]->type()   << "\t";
     file << mosCell[i]->center() << "\t";
     file << mos->type()          << "\t";
     file << mos->w()             << "\t";
     file << mos->l()             << "\t";
     file << mos->m()             << "\t";

		 for( int j = 0 ; j < mosCell[i]->connect().size() ; j++ )
				file << mosCell[i]->connect()[j]->name() << "\t";

		 file << endl;
	}

	for( int i = 0 ; i < subcktCell.size() ; i++ )
	{
     SubcktModel *model = static_cast<SubcktNode*>( subcktCell[i] )->model();

     file << subcktCell[i]->name()    << "\t";
     file << subcktCell[i]->type()    << "\t";
     file << subcktCell[i]->center()  << "\t";
     file << model->name()            << "\t";

     for( int j = 0 ; j < subcktCell[i]->connect().size() ; j++ )
        file << subcktCell[i]->connect()[j]->name() << "\t";

     file << endl;
  }
  file << endl;

  for( int i = 0 ; i < subcktModel.size() ; i++ )
     writeSubcktModel( static_cast<SubcktModel*>( subcktModel[i] ) );
  file << endl;
}
