#include "ICPlacement.h"

#include <queue>
#include <algorithm>
#include <vector>
#include <float.h>
using namespace std;

#include "../Lib/Model/SubcktModel.h"
#include "../Lib/Model/MosModel.h"
#include "../Lib/Model/ICModel.h"
#include "../Lib/Component/Mos.h"
#include "../Lib/Node/SubcktNode.h"
#include "../Lib/Node/MosNode.h"
#include "../Lib/Node/NetNode.h"
#include "../Lib/TechFile/TechFile.h"

bool ICPlacement::placement()
{
  vector<Model*>  &subcktModels = m_model->model()->subcktModel();
  bool            success = true;

  for( int i = subcktModels.size() - 1 ; i >= 0  ; i-- )
  {
     SubcktModel *model = static_cast<SubcktModel*>( subcktModels[i] );

     if( model->model()->subcktCell().size() )
       success &= subcktPlacement ( model );
     else
       success &= mosPlacement    ( model );

     if( !success ) return false;
  }

  return success &= subcktPlacement( m_model );
}

bool ICPlacement::mosPlacement( SubcktModel *model )
{
  struct MosNodesInfo
  {
    int start;
    int end;
  };

  // calculate cost
  vector<Node*> &mosNodes = model->model()->mosCell();
  int           pmosNum   = 0;

  for( int i = 0 ; i < mosNodes.size() ; i++ )
  {
     int cost = 0;
     Mos *mos = static_cast<MosNode*>( mosNodes[i] )->model()->model();

     if( mos->type() == Mos::PMOS )
     {
       cost += ( mosNodes.size() << 2 );
       pmosNum++;
     }

     for( int j = 0 ; j < MosNode::PIN_NUM ; j++ )
     {
        NetNode *node = static_cast<NetNode*>( mosNodes[i]->connect()[j] );

        if( node->type() == Node::VDD || node->type() == Node::VSS )
          continue;

        cost += ( node->connect().size() - 1 );

        for( register int k = 0 ; k < j ; k++ )
           if( node->name() == mosNodes[i]->connect()[k]->name() )
           {
             cost -= ( node->connect().size() - 1 );
             break;
           }
     }
     mosNodes[i]->setCost( cost );
  }
  // end calculate cost
  // sort
  queue<MosNodesInfo> pipe;
  MosNodesInfo        init;

  init.start  = 0;
  init.end    = mosNodes.size() - 1;

  pipe.push( init );

  while( pipe.size() )
  {
    MosNodesInfo  info        = pipe.front();
    int           breakPoint  = info.start;
    int           pivot       = ( info.start + info.end ) >> 1;

    swap( mosNodes[pivot] , mosNodes[info.end] );
    pivot = info.end;

    for( int i = info.start ; i < info.end ; i++ )
       if( mosNodes[i]->cost() > mosNodes[pivot]->cost() )
       {
         swap( mosNodes[i] , mosNodes[breakPoint] );
         breakPoint++;
       }

    swap( mosNodes[breakPoint] , mosNodes[info.end] );

    MosNodesInfo  backInfo;

    backInfo.start  = breakPoint + 1;
    backInfo.end    = info.end;
    info.end        = breakPoint - 1;

    if( info.start      < info.end     ) pipe.push( info     );
    if( backInfo.start  < backInfo.end ) pipe.push( backInfo );

    pipe.pop();
  }
  // end sort

  // calculate cost
  for( int i = 0 ; i < mosNodes.size() ; i++ )
     mosNodes[i]->setCost( mosNodes.size() - i );
  // end calculate cost

  // rough placement
  queue<Node*>  placeQueue;
  int           VISIT       = mosNodes[0]->visit() + 1;
  int           minCost     = mosNodes.size() - pmosNum;
  int           prx         = 1;  // pmos right x
  int           plx         = -1; // pmos left  x
  int           nrx         = 1;  // nmos right x
  int           nlx         = -1; // nmos left  x
  bool          placeNmos   = false;

  mosNodes[0]->setCenter( 0 , 0 );
  mosNodes[0]->setVisit( VISIT );

  placeQueue.push( mosNodes[0] );

  while( placeQueue.size() )
  {
    Node* node        = placeQueue.front();
    bool  placePRight = true;
    bool  placeNRight = true;

    for( int i = 0 ; i < node->connect().size() ; i++ )
    {
       vector<Node*> *net = &node->connect()[i]->connect();

       if( !is_sorted( net->begin() , net->end() ,
                        static_cast<bool (*)( Node* , Node* )>
                        ( Node::costCompare ) ) )
         sort(  net->begin() , net->end() ,
                static_cast<bool (*)( Node* , Node* )>
                ( Node::costCompare ) );

       for( int i = 0 ; i < net->size() ; i++ )
          if( net->at( i )->visit() != VISIT )
          {
            int x;
            int y;

            net->at(i)->setVisit( VISIT );
            placeQueue.push( net->at( i ) );

            if( net->at( i )->cost() > minCost )
            {
              x           = ( placePRight ) ? prx++ : plx--;
              y           = 0;
              placePRight = !placePRight;
            }
            else
            {
              y = -1;

              if( placeNmos )
              {
                x           = ( placeNRight ) ? nrx++ : nlx--;
                placeNRight = !placeNRight;
              }
              else
              {
                x         = 0;
                placeNmos = true;
              }
            }
            net->at( i )->setCenter( x , y );
          }
    }
    placeQueue.pop();
  }
  // end rough placement

  if( !tech ) return false;

  // detail placement
  double  metal2Width = tech->rule( SpacingRule::MIN_WIDTH   ,  "METAL2" );
  double  metal2Space = tech->rule( SpacingRule::MIN_SPACING ,  "METAL2" );
  double  nimpSpace   = tech->rule( SpacingRule::MIN_SPACING ,  "NIMP"   );
  double  pimpSpace   = tech->rule( SpacingRule::MIN_SPACING ,  "PIMP"   );
  double  conAndDiff  = tech->rule( SpacingRule::MIN_SPACING ,  "CONT" ,
                                                                "DIFF" );
  Mos     *pmos       = static_cast<MosNode*>( mosNodes[0]        )->model()
                        ->model();
  Mos     *nmos       = static_cast<MosNode*>( mosNodes[pmosNum]  )->model()
                        ->model();
  double  mosWidth    = max(  pmos->implant().width() + pimpSpace ,
                              nmos->implant().width() + nimpSpace );
  double  channel     = ( model->model()->ioNum() - 2 +
                          model->model()->netNum() + 1 ) *
                        ( metal2Width + metal2Space ) - metal2Space;
  double  p2n         = channel + 2 * conAndDiff +
                        ( pmos->diffusion().height() +
                          nmos->diffusion().height() ) / 2;
  double  height      = ( pmos->implant().height() +
                          nmos->implant().height() ) / 2 +
                        p2n + ( nimpSpace + pimpSpace ) / 2;
  double  width       = max(  pmosNum , static_cast<int>( mosNodes.size() -
                              pmosNum ) ) * mosWidth + 2 * channel;
  double  xbias       = ( pmosNum & 1 ) ? 0 : -mosWidth / 2 ;
  double  ybias       = ( height - pimpSpace - pmos->implant().height() )
                        / 2;
  

  for( int i = 0 ; i < mosNodes.size() ; i++ )
  {
     double x = xbias + mosNodes[i]->center().x() * mosWidth;
     double y = ybias + mosNodes[i]->center().y() * p2n;

     mosNodes[i]->setCenter( x , y );
  }
  
  model->setHeight( height  );
  model->setWidth ( width   );
  // end detail placement
  
  return true;
}

bool ICPlacement::subcktPlacement( SubcktModel *model )
{
  enum Direct
  {
    RIGHT,
    RIGHT_BOTTOM,
    BOTTOM,
    LEFT_BOTTOM,
    LEFT,
    LEFT_TOP,
    TOP,
    RIGHT_TOP,
    MAX_DIRECT
  };

  vector<Node*> &subcktNodes  = model->model()->subcktCell();

  // caculate cost
  for( int i = 0 ; i < subcktNodes.size() ; i++ )
  {
     vector<Node*>  &nets   = subcktNodes[i]->connect();
     SubcktModel    *subckt = static_cast<SubcktNode*>( subcktNodes[i] )
                              ->model();
     int            cost    = 0;
  
     subcktNodes[i]->setHeight( subckt->height()  );
     subcktNodes[i]->setWidth ( subckt->width()   );
     
     for( int j = 0 ; j < nets.size() ; j++ )
     {
        if( nets[j]->type() == Node::VDD || nets[j]->type() == Node::VSS )
          continue;

        cost += nets[j]->connect().size() - 1;
        
        for( int k = 0 ; k < j ; k++ )
           if( nets[j]->name() == nets[k]->name() )
           {
             cost -= nets[j]->connect().size() - 1;
             break;
           }
     }
     subcktNodes[i]->setCost( cost );
  }
  // end caculate cost
  
  sort( subcktNodes.begin() , subcktNodes.end() ,
        static_cast<bool (*)( Node* , Node* )>( Node::costCompare ) );

  // rough placement
  vector<Point> plane;
  queue<Node*>  placeQueue;
  int           VISIT = subcktNodes[0]->visit() + 1;
  int           xMin  = 0;
  int           yMin  = 0;
  
  subcktNodes[0]->setCenter ( 0 , 0 );
  subcktNodes[0]->setVisit  ( VISIT );
  
  placeQueue.push( subcktNodes[0] );
  
  while( placeQueue.size() )
  {
    Node          *node   = placeQueue.front();
    int           layer   = 1;
    int           direct  = 0;
    
    for( int i = 0 ; i < node->connect().size() ; i++ )
    {
       vector<Node*> &net = node->connect()[i]->connect();
    
       if( !is_sorted(  net.begin() , net.end() ,
                        static_cast<bool (*)( Node* , Node* )>
                        ( Node::costCompare ) ) )
         sort(  net.begin() , net.end() ,
                static_cast<bool (*)( Node* , Node* )>
                ( Node::costCompare ) );

       for( int j = 0 ; j < net.size() ; j++ )
          if( net[j]->visit() != VISIT )
          {
            net[j]->setVisit( VISIT );
            placeQueue.push( net[j] );
            
            do
            {
              int x = node->center().x();
              int y = node->center().y();
            
              if( RIGHT_TOP <= direct || direct <= RIGHT_BOTTOM )
              {
                x += layer;
              }
              if( LEFT_BOTTOM <= direct && direct <= LEFT_TOP )
              {
                x -= layer;
                if( x < xMin ) xMin = x;
              }
              if( RIGHT_BOTTOM <= direct && direct <= LEFT_BOTTOM )
              {
                y -= layer;
                if( y < yMin ) yMin = y;
              }
              if( LEFT_TOP <= direct && direct <= RIGHT_TOP )
              {
                y += layer;
              }

              net[j]->setCenter( x , y );
              if( direct < MAX_DIRECT ) direct++;
              else
              {
                direct = RIGHT;
                layer++;
              }
              
            }while( find( plane.begin () ,
                          plane.end   () ,
                          net[j]->center() ) != plane.end() );
            
            plane.push_back( net[j]->center() );
          }
    }
    placeQueue.pop();
  }
  
  sort( subcktNodes.begin() , subcktNodes.end() ,
        []( Node *front , Node *back )
        { return (  front->center().y() < back->center().y() ||
                    front->center().x() < back->center().x() ); } );

  int x = xMin;
  int y = yMin;

  for( int i = 0 ; i < subcktNodes.size() ; i++ )
  {
     if( y < subcktNodes[i]->center().y() )
     {
       y++;
       x = xMin;
     }
     subcktNodes[i]->setCenter( x , y );
     x++;
  }

  sort( subcktNodes.begin() , subcktNodes.end() ,
        []( Node *front , Node *back )
        { return (  front->center().x() < back->center().x() ||
                    front->center().y() < back->center().y() ); } );

  x = xMin;
  y = yMin;

  for( int i = 0 ; i < subcktNodes.size() ; i++ )
  {
     if( x < subcktNodes[i]->center().x() )
     {
       x++;
       y = yMin;
     }
     subcktNodes[i]->setCenter( x , y );
     y++;
  }
  
  // end rough placement

  // detial placement
  double xMax   = 0;
  double yMax   = 0;
  double localX;
  double localY;

  sort( subcktNodes.begin() , subcktNodes.end() ,
        []( Node *front , Node *back )
        { return (  front->center().y() < back->center().y() ||
                    front->center().x() < back->center().x() ); } );

  plane.clear();

  vector<Point> &contour = plane;

  contour.push_back( Point( 0       , 0 ) );
  contour.push_back( Point( DBL_MAX , 0 ) );

  for( int i = 0 ; i < subcktNodes.size() ; i++ )
  {
     int    front   = -1;
     int    end     = contour.size() - 1;
     double localXr;
     double halfH   = subcktNodes[i]->height()  / 2;
     double halfW   = subcktNodes[i]->width ()  / 2;
     
     if( subcktNodes[i]->center().x() == xMin )  localX = localXr = 0;
     
     localY   =   0;
     localXr  +=  subcktNodes[i]->width();
  
     for( int j = 0 ; j < contour.size() ; j++ )
     {
        if( front == -1 )
        {
          if      ( contour[j].x() >  localX )  front = j;
          else if ( contour[j].x() == localX )  front = j + 1;
          else                                  continue;
        }
        if( localX < contour[j].x() && contour[j].x() < localXr     ||
            contour[j].x() == localX  && contour[j+1].x() != localX ||
            contour[j].x() == localXr && contour[j+1].x() == localXr )
        {
          if( contour[j].y() > localY ) localY  = contour[j].y();
        }
        if( localXr < contour[j].x() && end == contour.size() - 1 )
        {
          end = j;
          break;
        }
        else if ( localXr == contour[j].x() && end == contour.size() - 1 )
        {
          end = j + 1;
          break;
        }
     }
     localY += halfH;
     localX += halfW;
     subcktNodes[i]->setCenter( localX , localY );
     localY += halfH;
     localX += halfW;
     if( localY > yMax ) yMax = localY;
     if( localX > xMax ) xMax = localX;
     
     contour.erase  ( contour.begin() + front , contour.begin() + end );
     contour.insert ( contour.begin() + front ,
     {  Point( subcktNodes[i]->left () , subcktNodes[i]->top    () ) ,
        Point( subcktNodes[i]->right() , subcktNodes[i]->top    () ) ,
        Point( subcktNodes[i]->right() , subcktNodes[i]->bottom () ) } );
  }

  double  height = yMax;
  double  width  = xMax;
  Point   bias( - width / 2 , - height / 2 );
  
  for( int i = 0 ; i < subcktNodes.size() ; i++ )
     subcktNodes[i]->setCenter( bias + subcktNodes[i]->center() );

  model->setCenter( 0 , 0   );
  model->setHeight( height  );
  model->setWidth ( width   );
  // end detial placement
  
  return true;
}
