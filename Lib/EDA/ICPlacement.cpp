#include "ICPlacement.h"

#include <queue>
#include <algorithm>
#include <vector>
#include <float.h>
using namespace std;

#include "../Model/CircuitModel.h"
#include "../Model/MosModel.h"
#include "../Component/Circuit.h"
#include "../Component/Mos.h"
#include "../Node/CircuitNode.h"
#include "../Node/MosNode.h"
#include "../Node/NetNode.h"
#include "../TechFile/TechFile.h"

bool ICPlacement::placement( CircuitModel *model )
{
  if( model->circuitCell().size() ) return circuitPlacement ( model );
  else                              return mosPlacement     ( model );
}

bool ICPlacement::mosPlacement( CircuitModel *model )
{
  // calculate cost
  vector<Node*> &mosNodes = model->mosCell();
  int           pmosNum   = 0;

  for( Node *mosNode : mosNodes )
  {
     int cost = 0;
     Mos *mos = static_cast<MosNode*>( mosNode )->model();

     if( mos->type() == Mos::PMOS )
     {
       cost += ( mosNodes.size() << 2 );
       pmosNum++;
     }

     for( int j = 0 ; j < MosNode::PIN_NUM ; j++ )
     {
        NetNode *node = static_cast<NetNode*>( mosNode->connect()[j] );

        if( node->type() == Node::VDD || node->type() == Node::VSS )
          continue;

        cost += ( node->connect().size() - 1 );

        for( register int k = 0 ; k < j ; k++ )
           if( node->name() == mosNode->connect()[k]->name() )
           {
             cost -= ( node->connect().size() - 1 );
             break;
           }
     }
     mosNode->setCost( cost );
  }
  // end calculate cost

  sort( mosNodes.begin() , mosNodes.end() ,
        static_cast<bool (*)( Node* , Node* )>( Node::costCompare ) );

  // calculate cost
  for( unsigned int i = 0 ; i < mosNodes.size() ; i++ )
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

    for( Node *netNode : node->connect() )
    {
       vector<Node*> &mos = netNode->connect();

       if( !is_sorted( mos.begin() , mos.end() ,
                        static_cast<bool (*)( Node* , Node* )>
                        ( Node::costCompare ) ) )
         sort(  mos.begin() , mos.end() ,
                static_cast<bool (*)( Node* , Node* )>
                ( Node::costCompare ) );

       for( Node *node : mos )
          if( node->visit() != VISIT )
          {
            int x;
            int y;

            node->setVisit( VISIT );
            placeQueue.push( node );

            if( node->cost() > minCost )
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
            node->setCenter( x , y );
          }
    }
    placeQueue.pop();
  }
  // end rough placement

  if( !tech ) return false;

  // detail placement
  double  metal2Width = tech->rule( SpacingRule::MIN_WIDTH   ,
                                    Layer::METAL2 );
  double  metal2Space = tech->rule( SpacingRule::MIN_SPACING ,
                                    Layer::METAL2 );
  double  nimpSpace   = tech->rule( SpacingRule::MIN_SPACING ,
                                    Layer::NIMPLANT );
  double  pimpSpace   = tech->rule( SpacingRule::MIN_SPACING ,
                                    Layer::PIMPLANT );
  double  conAndDiff  = tech->rule( SpacingRule::MIN_SPACING ,
                                    Layer::CONTACT , Layer::DIFFUSION );
  Mos     *pmos       = static_cast<MosNode*>( mosNodes[0]        )->model();
  Mos     *nmos       = static_cast<MosNode*>( mosNodes[pmosNum]  )->model();
  double  mosWidth    = max(  pmos->implant().width() + pimpSpace ,
                              nmos->implant().width() + nimpSpace );
  double  channel     = ( model->ioNum() - 2 + model->netNum() + 1 ) *
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
  double  ybias       = ( height - pimpSpace - pmos->implant().height() ) / 2;

  for( Node *mosNode : mosNodes )
  {
     double x = xbias + mosNode->center().x() * mosWidth;
     double y = ybias + mosNode->center().y() * p2n;

     mosNode->setCenter( x , y );
  }

  model->setHeight( height  );
  model->setWidth ( width   );
  // end detail placement

  return true;
}

bool ICPlacement::circuitPlacement( CircuitModel *model )
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

  vector<Node*> &circuitNodes = model->circuitCell();

  // caculate cost
  for( Node *node : circuitNodes )
  {
     vector<Node*>  &nets     = node->connect();
     CircuitModel   *circuit  = static_cast<CircuitNode*>( node )->model();
     int            cost      = 0;
  
     node->setHeight( circuit->height ()  );
     node->setWidth ( circuit->width  ()  );
     
     for( unsigned int j = 0 ; j < nets.size() ; j++ )
     {
        if( nets[j]->type() == Node::VDD || nets[j]->type() == Node::VSS )
          continue;

        cost += nets[j]->connect().size() - 1;
        
        for( unsigned int k = 0 ; k < j ; k++ )
           if( nets[j]->name() == nets[k]->name() )
           {
             cost -= nets[j]->connect().size() - 1;
             break;
           }
     }
     node->setCost( cost );
  }
  // end caculate cost
  
  sort( circuitNodes.begin() , circuitNodes.end() ,
        static_cast<bool (*)( Node* , Node* )>( Node::costCompare ) );

  // rough placement
  vector<Point> plane;
  queue<Node*>  placeQueue;
  int           VISIT = circuitNodes[0]->visit() + 1;
  int           xMin  = 0;
  int           yMin  = 0;

  circuitNodes[0]->setCenter( 0 , 0 );
  circuitNodes[0]->setVisit ( VISIT );

  placeQueue.push( circuitNodes[0] );

  while( placeQueue.size() )
  {
    Node          *node   = placeQueue.front();
    int           layer   = 1;
    int           direct  = 0;

    for( Node *netNode : node->connect() )
    {
       vector<Node*> &subckt = netNode->connect();

       if( !is_sorted(  subckt.begin() , subckt.end() ,
                        static_cast<bool (*)( Node* , Node* )>
                        ( Node::costCompare ) ) )
         sort(  subckt.begin() , subckt.end() ,
                static_cast<bool (*)( Node* , Node* )>
                ( Node::costCompare ) );

       for( Node *subcktNode : subckt )
          if( subcktNode->visit() != VISIT )
          {
            subcktNode->setVisit( VISIT );
            placeQueue.push( subcktNode );

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

              subcktNode->setCenter( x , y );
              if( direct < MAX_DIRECT ) direct++;
              else
              {
                direct = RIGHT;
                layer++;
              }
              
            }while( find( plane.begin () ,
                          plane.end   () ,
                          subcktNode->center() ) != plane.end() );
            
            plane.push_back( subcktNode->center() );
          }
    }
    placeQueue.pop();
  }
  
  sort( circuitNodes.begin() , circuitNodes.end() ,
        []( Node *front , Node *back )
        { return (  front->center().y() < back->center().y() ||
                    front->center().x() < back->center().x() ); } );

  int x = xMin;
  int y = yMin;

  for( Node *node : circuitNodes )
  {
     if( y < node->center().y() )
     {
       y++;
       x = xMin;
     }
     node->setCenter( x , y );
     x++;
  }

  sort( circuitNodes.begin() , circuitNodes.end() ,
        []( Node *front , Node *back )
        { return (  front->center().x() < back->center().x() ||
                    front->center().y() < back->center().y() ); } );

  x = xMin;
  y = yMin;

  for( Node *node : circuitNodes )
  {
     if( x < node->center().x() )
     {
       x++;
       y = yMin;
     }
     node->setCenter( x , y );
     y++;
  }
  // end rough placement

  // detial placement
  double xMax   = 0;
  double yMax   = 0;
  double localX;
  double localY;

  sort( circuitNodes.begin() , circuitNodes.end() ,
        []( Node *front , Node *back )
        { return (  front->center().y() < back->center().y() ||
                    front->center().x() < back->center().x() ); } );

  plane.clear();

  vector<Point> &contour = plane;

  contour.push_back( Point( 0       , 0 ) );
  contour.push_back( Point( DBL_MAX , 0 ) );

  for( Node *node : circuitNodes )
  {
     int          front   = -1;
     unsigned int end     = contour.size() - 1;
     double       localXr;
     double       halfH   = node->height()  / 2;
     double       halfW   = node->width ()  / 2;

     if( node->center().x() == xMin )  localX = localXr = 0;

     localY   =   0;
     localXr  +=  node->width();

     for( unsigned int j = 0 ; j < contour.size() ; j++ )
     {
        if( front == -1 )
        {
          if      ( contour[j].x() >  localX )  front = j;
          else if ( contour[j].x() == localX )  front = j + 1;
          else                                  continue;
        }
        if( ( localX < contour[j].x() && contour[j].x() < localXr       ) ||
            ( contour[j].x() == localX  && contour[j+1].x() != localX   ) ||
            ( contour[j].x() == localXr && contour[j+1].x() == localXr  ) )
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
     node->setCenter( localX , localY );
     localY += halfH;
     localX += halfW;
     if( localY > yMax ) yMax = localY;
     if( localX > xMax ) xMax = localX;

     contour.erase  ( contour.begin() + front , contour.begin() + end );
     contour.insert ( contour.begin() + front ,
     {  Point( node->left () , node->top    () ) ,
        Point( node->right() , node->top    () ) ,
        Point( node->right() , node->bottom () ) } );
  }

  double  height = yMax;
  double  width  = xMax;
  Point   bias( - width / 2 , - height / 2 );
  
  for( Node *node : circuitNodes ) node->setCenter( bias + node->center() );

  model->setCenter( 0 , 0   );
  model->setHeight( height  );
  model->setWidth ( width   );
  // end detial placement

  return true;
}