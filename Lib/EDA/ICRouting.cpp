#include "ICRouting.h"

#include <algorithm>
#include <fstream>
#include <cfloat>
#include <iomanip>
#include <queue>
#include <stack>
#include <climits>
#include <cmath>
#include <iostream>
#include <cassert>
using namespace std;

#include "../Model/CircuitModel.h"
#include "../Model/MosModel.h"
#include "../Node/MosNode.h"
#include "../Node/NetNode.h"
#include "../Node/CircuitNode.h"
#include "../Component/Circuit.h"
#include "../Component/Mos.h"
#include "../Component/Layer.h"
#include "../Component/ViaDevice.h"
#include "../Graphic/Rectangle.h"
#include "../TechFile/TechFile.h"

bool ICRouting::routing( CircuitModel *model )
{
  if( model ) circuitModel = model;
  else        return false;

  if( model->circuitCell().size() ) return gridRouting    ();
  else                              return channelRouting ();
}

bool ICRouting::channelRouting()
{
  channelCost   ();
  channelRough  ();
  channelDetail ();

  // clear data
  nets.clear();
  vcg.clear();
  // end clear data

  return true;
}

bool ICRouting::gridRouting()
{
  gridCost  ();
  gridRough ();
  gridDetail();

  // clear data
  blocks.clear();
  nets.clear();
  ioPos.clear();
  // end clear data

  return true;
}

void ICRouting::channelCost()
{
  vector<Node*> &mosNodes = circuitModel->mosCell();

  // set number 設定編號
  sort( mosNodes.begin() , mosNodes.end() ,
        []( Node *front , Node *back )
        {
          if( front->center().y() == back->center().y() )
            return front->center().x() < back->center().x();
          return front->center().y() > front->center().y();
        } );

  NMOS_BIAS  = -1;
  PMOS_FIRST = 0;

  for( unsigned int i = 0 ; i < mosNodes.size() ; i++ )
  {
     mosNodes[i]->setCost( i ); // use number as cost

     // find nmos bias
     if(  NMOS_BIAS == -1 &&
          static_cast<MosNode*>( mosNodes[i] )->model()->Mos::type()
          == Mos::NMOS )
     {
       NMOS_BIAS = i;

       // setup first nmos and pmos num
       if(  mosNodes[PMOS_BIAS]->center().x() <=
            mosNodes[NMOS_BIAS]->center().x() )
       {
         PMOS_FIRST = 0;
         for( unsigned int j = PMOS_BIAS ; j < i ; j++ )
            if( mosNodes[j]->center().x() ==
                mosNodes[NMOS_BIAS]->center().x() )
            {
              NMOS_FIRST = j;
              break;
            }
       }
       else
       {
         NMOS_FIRST = 0;
         for( unsigned int j = NMOS_BIAS + 1 ; j < mosNodes.size() ; j++ )
            if( mosNodes[j]->center().x() ==
                mosNodes[PMOS_BIAS]->center().x() )
            {
              PMOS_FIRST = j - NMOS_BIAS;
              break;
            }
       }
       // end setup first nmos and pmos num
     }
     // end find nmos bias
  }
  // end set number 設定編號

  // set cost
  for( Node *node : circuitModel->io() )
     if( node->type() != Node::VDD && node->type() != Node::VSS )
     {
       node->setCost( node->connect().size() );
       nets.push_back( static_cast<NetNode*>( node ) );
     }
     else
       node->setCost( -1 );

  for( Node *node : circuitModel->net() )
  {
     node->setCost( node->connect().size() );
     nets.push_back( static_cast<NetNode*>( node ) );
  }
  // end set cost
  
  sort( nets.begin() , nets.end() ,
        static_cast<bool (*)( Node* , Node* )>( Node::costCompare ) );

  const int PINS_PER_MOS  = MosNode::PIN_NUM - 1;

  MAX_PIN_NUM = 0;

  // set HCG
  for( unsigned int i = 0 ; i < nets.size() ; i++ )
  {
     vector<Node*>  &mos = nets[i]->connect();
     Layer          net;

     nets[i]->setCost( i );
     net.setHeadPin( -1 );

     sort(  mos.begin() , mos.end() , []( Node* front , Node* back )
            {
              if( front->center().x() == back->center().x() )
                return front->center().y() > back->center().y();
              return front->center().x() < back->center().x();
            } );

     // set net segment
     for( Node *node : mos )
     {
        int index[PINS_PER_MOS];
        int pinIndex  = 0;
        int BIAS;
        int CONNECT_MOS;

        // setup constant
        if( node->cost() >= NMOS_BIAS && NMOS_BIAS != -1 )
        {
          BIAS        = -NMOS_BIAS + NMOS_FIRST;
          CONNECT_MOS = Mos::NMOS;
        }
        else
        {
          BIAS        = PMOS_FIRST;
          CONNECT_MOS = Mos::PMOS;
        }
        // end setup constant
        net.setHeight( CONNECT_MOS );

        // setup pins connect to mos and find max pin num
        for( int j = MosNode::D ; j <= MosNode::S ; j++ )
           if( node->connect()[j] == nets[i] )
           {
             index[pinIndex]  = ( node->cost() + BIAS ) * PINS_PER_MOS + j;
             MAX_PIN_NUM      = max( MAX_PIN_NUM , index[pinIndex] );
             pinIndex++;
           }
        // end setup pins connect to mos and find max pin num

        // setup segments
        for( int j = 0 ; j < pinIndex ; j++ )
        {
           if( net.headPin() != -1  )
           {
             net.setTailPin( index[j] );
             nets[i]->segments().push_back( net );
           }
           net.setHeadPin( index[j] );
           net.setWidth  ( CONNECT_MOS );
        }
        // end setup segments
     }
     // end set net segment
  }
  
  // setup io pin
  for( NetNode *node : nets )
  {
     if( node->type() == Node::IO )
     {
       vector<Layer>  &segments = node->segments();
       Layer          segment;

       segment.setHeight( Mos::UNKNOWN );
       segment.setWidth ( Mos::UNKNOWN );

       if( segments.empty() )
       {
         segment.setPin( -1 , MAX_PIN_NUM );
         segments.push_back( segment );
       }
       else
       {
         segment.setPin( -1 , segments.front().headPin() );
         segments.insert( segments.begin() , segment );
         segment.setPin( segments.back().tailPin() , MAX_PIN_NUM + 1 );
         segments.push_back( segment );
       }
     }
  }
  // end setup io pin
  // end set HCG

  // set VCG
  vcg.resize( MAX_PIN_NUM + 1 , vector<int>( Mos::TYPE_NUM , Mos::UNKNOWN ) );

  if( NMOS_BIAS != -1 )
  {
    const int MIN_PIN_NUM = max( PMOS_FIRST , NMOS_FIRST ) * PINS_PER_MOS;
    int       VCG_BIAS    = MIN_PIN_NUM;

    for(  int i = MIN_PIN_NUM / PINS_PER_MOS ;
          i <= MAX_PIN_NUM / PINS_PER_MOS ; i++ )
    {
       vector<Node*> &nmosConnect = mosNodes[NMOS_BIAS+i]->connect();
       vector<Node*> &pmosConnect = mosNodes[i]->connect();

       for( int j = MosNode::D ; j <= MosNode::S ; j++ )
       {
          if( VCG_BIAS + j > MAX_PIN_NUM ) break;
          vcg[VCG_BIAS+j][Mos::NMOS] = nmosConnect[j]->cost();
          vcg[VCG_BIAS+j][Mos::PMOS] = pmosConnect[j]->cost();
       }
       VCG_BIAS += PINS_PER_MOS;
    }
  }
  // end set VCG
}

void ICRouting::channelRough()
{
  // rough routing
  vector<Layer>         intervals;
  vector<vector<Layer>> netInfo( nets.size() );

  // put intervals
  for( unsigned int i = 0 ; i < nets.size() ; i++ )
  {
     Layer interval;

     interval.setHeight( i );
     interval.setTrack ( -1 );

     for( Layer &net : nets[i]->segments() )
        for( int k = net.headPin() ; k < net.tailPin() ; k++ )
        {
           interval.setPin( k , k + 1 );
           intervals.push_back( interval );
           netInfo[i].push_back( interval );
        }
  }
  // end put intervals

  sort( intervals.begin() , intervals.end() ,
        []( const Layer &front , const Layer &back )
        {
          if( front.headPin() == back.headPin() )
            return front.height() < back.height();
          return front.headPin() < back.headPin();
        } );

  unsigned int routeNum = 0;

  // unrestricted left-edge routing
  for( track = 0 ; routeNum < intervals.size() ; track++ )
  {
     int mark         = -1;
     int lastNetNum   = 0;
     int routeNumDiff = 0;

     for( unsigned int i = 0 ; i < intervals.size() ; i++ )
     {
        int leftEdge  = static_cast<int>( intervals[i].headPin() );
        int rightEdge = static_cast<int>( intervals[i].tailPin() );
        int netNum    = static_cast<int>( intervals[i].height () );

        if( intervals[i].track() != -1 ) continue;
        if( leftEdge < mark ) continue;
        if( leftEdge == mark && mark != -1 && netNum != lastNetNum )
          continue;

        // check vertical constraint
        bool  constraint = true;
        int   segmentNum = 1;
        int   rightEdgeT;

        if( leftEdge != -1 )
        {
          if( vcg[leftEdge][Mos::PMOS] != -1 &&
              vcg[leftEdge][Mos::PMOS] != netNum ) continue;
        }
        if( rightEdge != MAX_PIN_NUM + 1 )
        {
          if( vcg[rightEdge][Mos::PMOS] != -1 &&
              vcg[rightEdge][Mos::PMOS] != netNum )
          {
            if( vcg[rightEdge][Mos::NMOS] == netNum ) continue;
            for( unsigned int j = 0 ; j < netInfo[netNum].size() ; j++ )
               if( netInfo[netNum][j].headPin() == rightEdge )
               {
                 for( unsigned int k = j ; k < netInfo[netNum].size() ; k++ )
                 {
                    if( netInfo[netNum][k].track() != -1 ) break;
                    rightEdgeT = netInfo[netNum][k].tailPin();

                    if( rightEdgeT != MAX_PIN_NUM + 1 &&
                        vcg[rightEdgeT][Mos::PMOS] != -1 &&
                        vcg[rightEdgeT][Mos::PMOS] != netNum )
                    {
                      if( vcg[rightEdgeT][Mos::NMOS] == netNum )  break;
                      else                                        continue;
                    }

                    constraint  =   false;
                    segmentNum  +=  k - j + 1;
                    break;
                 }
                 break;
               }
            if( constraint ) continue;
          }
        }
        // end check vertical constraint

        mark        = ( constraint ) ? rightEdge : rightEdgeT;
        lastNetNum  = netNum;

        int index = i;
        
        if( leftEdge != -1 )
        {
          if( vcg[leftEdge][Mos::PMOS] == netNum )
            vcg[leftEdge][Mos::PMOS] = -1;
          for( Layer &segment : netInfo[netNum] )
             if( segment.tailPin() == leftEdge && segment.track() == -1 )
             {
               vcg[leftEdge][Mos::PMOS] = netNum;
               break;
             }
        }

        for( int j = 0 ; j < segmentNum ; j++ )
        {
           intervals[index].setTrack( track );
           routeNum++;
           routeNumDiff++;

           for( Layer &segment : netInfo[netNum] )
              if( segment.headPin() == leftEdge &&
                  segment.tailPin() == rightEdge )
              {
                segment.setTrack( track );
                break;
              }

           if( constraint == false && j + 1 != segmentNum )
           {
             leftEdge++;
             rightEdge++;

             for( unsigned int k = index + 1 ; k < intervals.size() ; k++ )
                if( intervals[k].headPin() == leftEdge &&
                    intervals[k].height () == netNum )
                {
                  index = k;
                  break;
                }
           }
        }

        if( rightEdge != MAX_PIN_NUM + 1 )
        {
          if( vcg[rightEdge][Mos::PMOS] == netNum )
            vcg[rightEdge][Mos::PMOS] = -1;
          for( Layer &segment : netInfo[netNum] )
             if( segment.headPin() == rightEdge && segment.track() == -1 )
             {
               vcg[rightEdge][Mos::PMOS] = netNum;
               break;
             }
        }
     }
     if( routeNumDiff == 0 ) break;
  }
  // end unrestricted left-edge routing

  // merge segments
  sort( intervals.begin() , intervals.end() ,
        []( const Layer &front , const Layer &back )
        {
          if( front.height() == back.height() )
          {
            if( front.headPin() == back.headPin() )
              return front.track() < back.track();
            return front.headPin() < back.headPin();
          }
          return front.height() < back.height();
        } );

  int listIndex = 0;

  netInfo.clear();
  netInfo.resize( nets.size() );

  for( unsigned int i = 0 ; i < intervals.size() ; i++ )
  {
     Layer  segment       = intervals[i];
     int    leftEdge      = segment.headPin();
     int    rightEdge     = segment.tailPin();
     int    segmentTrack  = segment.track();
     int    netIndex      = segment.height();
     bool   merge         = true;

     // merge segments
     for( unsigned int j = i + 1 ; j < intervals.size() ; j++ )
     {
        for( Layer &segment : nets[netIndex]->segments() )
           if(  rightEdge == segment.headPin() ||
                rightEdge == segment.tailPin() )
             merge = false;
     
        if( merge )
        {
          if( intervals[j].height () == netIndex &&
              intervals[j].track  () == segmentTrack &&
              intervals[j].headPin() == rightEdge )
            rightEdge++;
          else
            merge = false;
        }

        if( !merge )
        {
          i = j - 1;
          break;
        }
     }
     segment.setTailPin( rightEdge );
     // end merge segments

     vector<Layer>  &netlist = nets[netIndex]->segments();
     Layer          layer;

     // connect segments
     for( unsigned int j = listIndex ; j < netlist.size() ; j++ )
     {
        if( netlist[j].headPin() == netlist[j].tailPin() )
        {
          layer.setType ( Layer::METAL1 );
          layer.setTrack( netlist[j].headPin() );
          layer.setPin  ( -1 , segmentTrack );
          netInfo[netIndex].push_back( layer );
          layer.setPin  ( segmentTrack , track );
          netInfo[netIndex].push_back( layer );
          listIndex++;
        }
        else if(  netlist[j].headPin()  <= leftEdge &&
                  leftEdge              <= netlist[j].tailPin() )
        {
          if( leftEdge == netlist[j].headPin() )
          {
            layer.setType ( Layer::METAL1 );
            layer.setTrack( leftEdge );
            
            switch( static_cast<int>( netlist[j].width() ) )
            {
              case Mos::NMOS: layer.setPin( segmentTrack , track );
                              netInfo[netIndex].push_back( layer ); break;
              case Mos::PMOS: layer.setPin( -1 , segmentTrack );
                              netInfo[netIndex].push_back( layer ); break;
            }
          }
          if( !netInfo[netIndex].empty() &&
              netInfo[netIndex].back().type() != Layer::METAL1 )
          {
            int lastTrack   = netInfo[netIndex].back().track();
            int topEdge     = min( lastTrack , segmentTrack );
            int bottomEdge  = max( lastTrack , segmentTrack );

            netInfo[netIndex].push_back(
              Layer( Layer::METAL1  , topEdge , bottomEdge , leftEdge ) );
          }
          netInfo[netIndex].push_back(
            Layer(  Layer::METAL2     , segment.headPin() ,
                    segment.tailPin() , segmentTrack ) );
          if( rightEdge >= netlist[j].tailPin() )
          {
            layer.setType ( Layer::METAL1 );
            layer.setTrack( rightEdge );

            switch( static_cast<int>( netlist[j].height() ) )
            {
              case Mos::NMOS: layer.setPin( segmentTrack , track );
                              netInfo[netIndex].push_back( layer ); break;
              case Mos::PMOS: layer.setPin( -1 , segmentTrack );
                              netInfo[netIndex].push_back( layer ); break;
            }
            listIndex++;
          }
          break;
        }
     }
     // connect segments

     if( i + 1 == intervals.size() || intervals[i+1].height() != netIndex )
     {
       netlist.clear();
       listIndex = 0;
     }
  }
  // end merge segments

  for( unsigned int i = 0 ; i < nets.size() ; i++ )
     if( nets[i]->segments().size() )
       netInfo[i].push_back(
       Layer( Layer::METAL1 , -1 , track ,
              nets[i]->segments().back().headPin() ) );

  for( unsigned int i = 0 ; i < netInfo.size() ; i++ )
     nets[i]->segments() = netInfo[i];

  // end rough routing
}

void ICRouting::channelDetail()
{
  vector<Node*> &mosNodes = circuitModel->mosCell();
  
  // detail routing
  double metal1Width  = tech->rule( SpacingRule::MIN_WIDTH    ,
                                    Layer::METAL1 );
  double metal1Space  = tech->rule( SpacingRule::MIN_SPACING  ,
                                    Layer::METAL1 );
  double metal2Width  = tech->rule( SpacingRule::MIN_WIDTH    ,
                                    Layer::METAL2 );
  double metal2Space  = tech->rule( SpacingRule::MIN_SPACING  ,
                                    Layer::METAL2 );
  double via12Width   = tech->rule( SpacingRule::MIN_WIDTH    ,
                                    Layer::VIA12  );
  double via12Space   = tech->rule( SpacingRule::MIN_SPACING  ,
                                    Layer::VIA12  );
  double conWidth     = tech->rule( SpacingRule::MIN_WIDTH    ,
                                    Layer::CONTACT );
  double conInPoly    = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                    Layer::POLY1 , Layer::CONTACT );
  double conInMetal   = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                    Layer::METAL1 , Layer::CONTACT );
  double viaInMetal1  = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                    Layer::METAL1 , Layer::VIA12 );
  double viaInMetal2  = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                    Layer::METAL2 , Layer::VIA12 );

  Mos *pmos = static_cast<MosNode*>( mosNodes[PMOS_BIAS] )->model();
  Mos *nmos = static_cast<MosNode*>( mosNodes[NMOS_BIAS] )->model();

  ViaDevice contact ( Layer::CONTACT  , 0 , 0 , 1 , 1 );
  ViaDevice via12   ( Layer::VIA12    , 0 , 0 , 1 , 1 );

  contact .setConWidth  ( conWidth );
  contact .setConInUpper( conInMetal , 0.06 );
  contact .setConInLower( conInPoly , conInPoly );
  via12   .setConWidth  ( via12Width );
  via12   .setConInUpper( viaInMetal2 , 0.06 );
  via12   .setConInLower( viaInMetal1 , 0.06 );
  
  contact.generate();
  via12.generate();

  double xUnit =  via12Width  + via12Space;
  double yUnit =  metal2Width + metal2Space;
  double yBias =  mosNodes[PMOS_BIAS]->center().y() +
                  pmos->source().upperLayer().bottom() - metal1Space - 0.06 -
                  metal2Width / 2;

  for( NetNode *node : nets )
  {
     vector<Layer>  &netlist    = node->segments();
     int            netlistSize = netlist.size();

     for( int j = 0 ; j < netlistSize ; j++ )
     {
        Layer   layer;
        double  x;
        double  y = yBias;
        double  height;
        double  width;
        bool    connectPmos = ( netlist[j].headPin() == -1    );
        bool    connectNmos = ( netlist[j].tailPin() == track );
        bool    connectGate = static_cast<int>( netlist[j].track() ) % 3 ==
                              MosNode::G;
        double  headFix     = ( connectPmos ) ? 1   : 0;
        double  tailFix     = ( connectNmos ) ? -1  : 0;
        int     mosIndex;

        switch( netlist[j].type() )
        {
          case Layer::METAL1:

            height    =   ( ( netlist[j].tailPin() + tailFix ) -
                            ( netlist[j].headPin() + headFix ) ) * yUnit +
                          metal2Width;
            if( height < 0 ) height = 0;
            width     =   metal1Width;
            mosIndex  =   static_cast<int>( netlist[j].track() ) / 3;
            x         =   mosNodes[mosIndex]->center().x() - xUnit +
                          static_cast<int>( netlist[j].track() ) % 3 * xUnit;
            y         -=  ( netlist[j].headPin() + headFix ) * yUnit +
                          ( height - metal2Width ) / 2;
            
            layer.setType   ( Layer::METAL1 );
            layer.setWidth  ( metal1Width );
            layer.setCenterX( x );

            if( connectPmos && connectNmos )
            {
              if( connectGate )
              {
                contact.setCenter( x , yBias );
                node->contacts().push_back( contact );
                contact.setCenter( x ,  mosNodes[NMOS_BIAS]->center().y() +
                                        nmos->source().upperLayer().top() +
                                        metal1Space + 0.06 +
                                        metal2Width / 2 );
                node->contacts().push_back( contact );

                layer.setCenterY( mosNodes[PMOS_BIAS]->center().y() +
                                  pmos->source().upperLayer().bottom() -
                                  metal1Space );
                layer.setHeight ( layer.center().y() -
                                  ( mosNodes[NMOS_BIAS]->center().y() +
                                    nmos->source().upperLayer().top() +
                                    metal1Space ) );
              }
              else
              {
                layer.setCenterY( mosNodes[PMOS_BIAS]->center().y() +
                                  pmos->source().upperLayer().top() );
                layer.setHeight ( layer.center().y() -
                                  mosNodes[NMOS_BIAS]->center().y() -
                                  nmos->source().upperLayer().bottom() );
              }
              layer.setCenterY( layer.center().y() - layer.height() / 2 );
              netlist.push_back( layer );
              break;
            }
            else if( connectPmos )
            {
              if( connectGate )
              {
                contact.setCenter( x , yBias );
                node->contacts().push_back( contact );
              }
              else
              {
                layer.setHeight ( metal1Space + 0.06 +
                                  pmos->source().upperLayer().height() );
                layer.setCenterY( mosNodes[PMOS_BIAS]->center().y() +
                                  pmos->source().upperLayer().top() -
                                  layer.height() / 2 );
                netlist.push_back( layer );
              }
            }
            else if( connectNmos )
            {
              layer.setCenterY( yBias - ( track - 1 ) * yUnit -
                                metal2Width / 2 );
              if( connectGate )
              {
                contact.setCenter( x ,  mosNodes[NMOS_BIAS]->center().y() +
                                        nmos->source().upperLayer().top() +
                                        metal1Space + 0.06 +
                                        metal2Width / 2 );
                node->contacts().push_back( contact );
                layer.setHeight ( layer.center().y() -
                                  ( contact.center().y() +
                                  contact.upperLayer().top() ) );
              }
              else
              {
                layer.setHeight ( layer.center().y() -
                                  ( mosNodes[NMOS_BIAS]->center().y() +
                                  nmos->source().upperLayer().bottom() ) );
              }
              layer.setCenterY( layer.center().y() - layer.height() / 2 );
              netlist.push_back( layer );
            }
            break;

          case Layer::METAL2:

            height = metal2Width;
            if( netlist[j].tailPin() != MAX_PIN_NUM + 1 )
            {
              mosIndex  = static_cast<int>( netlist[j].tailPin() ) / 3;
              width     = mosNodes[mosIndex]->center().x() - xUnit +
                          static_cast<int>( netlist[j].tailPin() ) % 3 *
                          xUnit + metal2Width / 2;
            }
            else
            {
              circuitModel->minRect().setCenter( 0 , 0 );
              width     = circuitModel->minRect().right();
            }
            if( netlist[j].headPin() != -1 )
            {
              mosIndex  =   static_cast<int>( netlist[j].headPin() ) / 3;
              x         =   mosNodes[mosIndex]->center().x() - xUnit +
                            static_cast<int>( netlist[j].headPin() ) % 3 *
                            xUnit - metal2Width / 2;
            }
            else
            {
              circuitModel->minRect().setCenter( 0 , 0 );
              x         = circuitModel->minRect().left();
            }
            width -=  x;
            x     +=  width / 2;
            y     -=  netlist[j].track() * yUnit;

            if( netlist[j].headPin() != -1 )
            {
              via12.setCenter(  x - width / 2 + metal2Width / 2 , y );
              node->contacts().push_back( via12 );
            }
            if( netlist[j].tailPin() != MAX_PIN_NUM + 1 )
            {
              via12.setCenter(  x + width / 2 - metal2Width / 2 , y );
              node->contacts().push_back( via12 );
            }
            break;

          default: break;
        }
        netlist[j].setCenter( x , y   );
        netlist[j].setHeight( height  );
        netlist[j].setWidth ( width   );
     }
  }
  // end detail routing
}

void ICRouting::gridCost()
{
  static bool first = true;
  const char  *logName = "gridCostLog.txt";
  
  if( first )
  {
    debug.open( logName , ios::out );
    first = false;
  }
  else
    debug.open( logName , ios::out | ios::app );

  findMinimumChannel();
  
  int row = floor/*round*/( circuitModel->height() / rowUnit );
  int col = floor/*round*/( circuitModel->width () / colUnit );

  // setup grid maze
  rowUnit = circuitModel->height() / row;
  colUnit = circuitModel->width () / col;
  blocks.resize( row , vector<Block>( col ) );
  blocks.shrink_to_fit();
  // end setup grid maze
  
  setupObstacle();
  
  debug << "Model : " << circuitModel->name() << endl;
  debug << "row = " << row << " unit = " << rowUnit << endl;
  debug << "col = " << col << " unit = " << colUnit << endl;

  for( int i = row - 1 ; i >= 0 ; i-- )
  {
     for( int j = 0 ; j < col ; j++ ) debug << blocks[i][j].value;
     debug << endl;
  }
  debug.close();
}

void ICRouting::gridRough()
{
  static bool first = true;
  const char  *logName = "gridRoughLog.txt";

  if( first )
  {
    debug.open( logName , ios::out );
    first = false;
  }
  else
    debug.open( logName , ios::out | ios::app );

  // setup nets
  nets.reserve( circuitModel->netNum() );
  for( Node *node : circuitModel->io() )
     if( node->type() != Node::VDD && node->type() != Node::VSS )
       nets.push_back( static_cast<NetNode*>( node ) );

  for( Node *node : circuitModel->net() ) nets.push_back( static_cast<NetNode*>( node ) );

  nets.shrink_to_fit();
  // end setup nets

  ioPos.resize( nets.size() );
  ioPos.shrink_to_fit();

  int netIndex = 0;

  // route io pin
  for( NetNode *node : nets )
  {
     forward_list<Point> ios;

     setupIOPin( node , netIndex , ios );
     // global routing
     hadlocksMazeRouting( ios , netIndex , node );

     // route circuitModel io pin
     if( node->type() == Node::IO )
     {
       // find corner point
       Point  source;
       int    sideDist = INT_MAX;

       for( Point &ioPin : ios )
       {
          int sides[] = { static_cast<int>( blocks.size()     - 1 - ioPin.y() ) ,
                          static_cast<int>( blocks[0].size()  - 1 - ioPin.x() ) ,
                          static_cast<int>( ioPin.y() ) , static_cast<int>( ioPin.x() ) };

          for( int side : sides )
             if( sideDist > side )
             {
               sideDist = side;
               source   = ioPin;
             }
       }
       // end find corner point
       leeAlgorithm( source , node , netIndex );
     }
     // end route circuitModel io pin

     setupBlockCrossNum( node );

     for( Layer &layer : node->segments() ) debug << layer << endl;
     debug << endl;
     // end global routing

     debug << "netnum : " << netIndex << " net : " << node->name() << endl;
     for( unsigned int i = blocks.size() - 1 ; i >= 0 ; i-- )
     {
        for( unsigned int j = 0 ; j < blocks[i].size() ; j++ )
           switch( blocks[i][j].value )
           {
             case Block::SPACE:     debug << "  "; break;
             case Block::OBSTACLE:  debug << "█"; break;
             default:               debug << setw( 2 );
                                    debug << blocks[i][j].value; break;
           }
        debug << endl;
        if( i == 0 ) break;
     }
     debug << endl;

     for( Point &io : ios ) blocks[io.y()][io.x()].value = Block::OBSTACLE;
     netIndex++;
  }
  // end route io pin
  
  debug << "Model : " << circuitModel->name() << endl;
  for( unsigned int i = blocks.size() - 1 ; i >= 0 ; i-- )
  {
     for( unsigned int j = 0 ; j < blocks[i].size() ; j++ )
        switch( blocks[i][j].value )
        {
          case Block::SPACE:     debug << "  "; break;
          case Block::OBSTACLE:  debug << "█"; break;
          default:               debug << setw( 2 );
                                 debug << blocks[i][j].value; break;
        }
     debug << endl;
     if( i == 0 ) break;
  }
  debug << endl;
  
  debug.close();
}

void ICRouting::gridDetail()
{
  static bool first     = true;
  const char  *logName  = "gridDetailLog.txt";
  
  if( first )
  {
    debug.open( logName , ios::out );
    first = false;
  }
  else
    debug.open( logName , ios::out | ios::app );

  double metal2Width  = tech->rule( SpacingRule::MIN_WIDTH    , Layer::METAL2 );
  double metal2Space  = tech->rule( SpacingRule::MIN_SPACING  , Layer::METAL2 );
  double via12Width   = tech->rule( SpacingRule::MIN_WIDTH    , Layer::VIA12  );
  double via12Space   = tech->rule( SpacingRule::MIN_SPACING  , Layer::VIA12  );
  double xUnit        = via12Width  + via12Space;
  double yUnit        = metal2Width + metal2Space;

  const Point   acceptCrossNum( floor( colUnit / xUnit ) , floor( rowUnit / yUnit ) );
  queue<Point>  waitedBlocks;
  
  setupCrossNet ( acceptCrossNum , waitedBlocks );
  moveNets      ( acceptCrossNum , waitedBlocks );
  setupNets     ( xUnit , yUnit );

  debug << "circuitModel : " << circuitModel->name() << endl;

  for( NetNode *node : nets )
  {
     debug << "Net : " << node->name() << endl;

     for( Layer &layer : node->segments() ) debug << layer << endl;
     debug << endl;
  }
  
  debug.close();
}

void ICRouting::findMinimumChannel()
{
  double metal2Width  = tech->rule( SpacingRule::MIN_WIDTH    , Layer::METAL2 );
  double metal2Space  = tech->rule( SpacingRule::MIN_SPACING  , Layer::METAL2 );
  double via12Width   = tech->rule( SpacingRule::MIN_WIDTH    , Layer::VIA12  );
  double via12Space   = tech->rule( SpacingRule::MIN_SPACING  , Layer::VIA12  );
  double xUnit        = via12Width  + via12Space;
  double yUnit        = metal2Width + metal2Space;

  // find minimum channel
  /*rowUnit = DBL_MAX;
  colUnit = DBL_MAX;
  
  for( Node *node : circuitModel->circuitCell() )
  {
     CircuitModel *circuitModel = static_cast<CircuitNode*>( node )->model();

     Point channel( ( circuitModel->width () -
                      circuitModel->minRect().width() ) / 2 ,
                    ( circuitModel->height() -
                      circuitModel->minRect().height() ) / 2 );

     if( rowUnit > channel.y() ) rowUnit = channel.y();
     if( colUnit > channel.x() ) colUnit = channel.x();
  }*/

  /*if( rowUnit < yUnit )*/ rowUnit = yUnit;
  /*if( colUnit < xUnit )*/ colUnit = xUnit;
  // end find minimum channel
}

void ICRouting::setupObstacle()
{
  // setup obstacle
  for( Node *node : circuitModel->circuitCell() )
  {
     Rectangle &minRect = static_cast<CircuitNode*>( node )->model()->minRect();

     minRect.setCenter( node->center() );

     double hHalf = circuitModel->height() / 2;
     double wHalf = circuitModel->width () / 2;

     int yMin = round( ( minRect.bottom() + hHalf ) / rowUnit );
     int yMax = round( ( minRect.top   () + hHalf ) / rowUnit );
     int xMin = round( ( minRect.left  () + wHalf ) / colUnit );
     int xMax = round( ( minRect.right () + wHalf ) / colUnit );

     if( yMax + 1 == static_cast<int>( blocks.size()    ) ) yMax--;
     if( xMax + 1 == static_cast<int>( blocks[0].size() ) ) xMax--;

     debug  << node->name() << " "
            << static_cast<CircuitNode*>( node )->model()->name() << endl;
     debug  << "y : " << yMin << " ~ " << yMax << endl;
     debug  << "x : " << xMin << " ~ " << xMax << endl;

     for( int i = yMin ; i <= yMax ; i++ )
        for( int j = xMin ; j <= xMax ; j++ )
           blocks[i][j].value = Block::OBSTACLE;
  }
  // end setup obstacle
}

void ICRouting::setupIOPin( NetNode *node , int netIndex , forward_list<Point> &ios )
{
  // setup io pin
  for( Node *connect : node->connect() )
  {
     CircuitNode   *circuitNode  = static_cast<CircuitNode*>( connect );
     CircuitModel  *model        = circuitNode->model();

     const int index = connect->searchConnectNode( node->name() );
     NetNode   *net  = static_cast<NetNode*>( model->io()[index] );

     const Layer *top    = nullptr;
     const Layer *bottom = nullptr;
     const Layer *left   = nullptr;
     const Layer *right  = nullptr;

     net->setVisit( 0 );

     // find four side
     for( const Layer &segment : net->segments() )
     {
        if( !top     || top->top()       < segment.top    () ) top    = &segment;
        if( !bottom  || bottom->bottom() > segment.bottom () ) bottom = &segment;
        if( !left    || left->left()     > segment.left   () ) left   = &segment;
        if( !right   || right->right()   < segment.right  () ) right  = &segment;
     }
     // end find four side

     bool          circuitCells  = model->circuitCell().size();
     vector<Node*> cells         = ( circuitCells ) ? model->circuitCell() :
                                                      model->mosCell();
     // check if io blocked
     for( Node *node : cells )
     {
        Rectangle minRect;

        if( circuitCells ) minRect = static_cast<CircuitNode*>( node )->model()->minRect();
        else               minRect = static_cast<MosNode*>    ( node )->model()->implant();

        minRect.setCenter( node->center() );

        if(  top && top->top() < minRect.top() &&
             top->left () < minRect.right() && top->right() > minRect.left () )
          top = nullptr;
        if(  bottom && bottom->bottom() > minRect.bottom() &&
             bottom->left() < minRect.right() && bottom->right() > minRect.left () )
          bottom = nullptr;
        if(  left && left->left() > minRect.left() &&
             left->top() > minRect.bottom() && left->bottom() < minRect.top() )
          left = nullptr;
        if(  right && right->right() < minRect.right() &&
             right->top() > minRect.bottom() && right->bottom() < minRect.top() )
          right = nullptr;
     }
     // end check if io blocked

     const Layer *layers[] = { top , bottom , left , right };

     for( const Layer *layer : layers )
     {
        if( !layer ) continue;

        Layer segment = *layer;
        Point ioPin;
        segment.setCenter( circuitNode->center() + layer->center() );

        if     ( layer == top     ) ioPin.set( segment.center().x() , segment.top() );
        else if( layer == bottom  ) ioPin.set( segment.center().x() , segment.bottom() );
        else if( layer == left    ) ioPin.set( segment.left() , segment.center().y() );
        else if( layer == right   ) ioPin.set( segment.right() , segment.center().y() );

        int row = round( ( ioPin.y() + circuitModel->height() / 2 ) / rowUnit );
        int col = round( ( ioPin.x() + circuitModel->width () / 2 ) / colUnit );

        if( row == static_cast<int>( blocks.size   () ) - 1 ) row--;
        if( col == static_cast<int>( blocks[0].size() ) - 1 ) col--;

        ioPos[netIndex].push_front( ioPin );
        blocks[row][col].value      = netIndex;
        blocks[row][col].connectNet = net;
        blocks[row][col].detour     = 0;
        ios.push_front( Point( col , row ) );
     }
  }
  // end setup io pin
}

void ICRouting::hadlocksMazeRouting(  forward_list<Point> &ios , int netIndex ,
                                      NetNode *node )
{
  Point source = ios.front();

  for( Point &target : ios )
  {
     if( target == source ) continue;
     if( blocks[target.y()][target.x()].connectNet->visit() == 0 &&
         blocks[source.y()][source.x()].connectNet !=
         blocks[target.y()][target.x()].connectNet )
     {
       enum Direction
       {
         UP,
         DOWN,
         LEFT,
         RIGHT,
         DIRECT_NUM
       };
     
       queue<Point> sources;
       queue<Point> nextDetour;
       Rectangle    rect( ( source.x() + target.x() ) / 2 , ( source.y() + target.y() ) / 2 ,
                          abs( source.y() - target.y() ) , abs( source.x() - target.x() ) );

       const int MD         = manhattanDistance( source , target );
       const int MAX_DETOUR = max(  max( blocks.size() - 1 - rect.top() ,
                                         blocks[0].size() - 1 - rect.right() ) ,
                                    max(  rect.bottom() , rect.left() ) );
       int  detour;
       int  direction;
       bool targeted = false;

       sources.push( source );

       // find path
       for( detour = 0 ; detour <= MAX_DETOUR ; detour++ )
       {
          while( !sources.empty() )
          {
            for( int direct = UP ; direct <= RIGHT ; direct++ )
            {
               unsigned int row = sources.front().y();
               unsigned int col = sources.front().x();

               switch( direct )
               {
                 case UP:

                   if( row + 1 == blocks.size() ) continue;
                   row++;
                   break;

                 case DOWN:

                   if( row == 0 ) continue;
                   row--;
                   break;

                 case LEFT:

                   if( col == 0 ) continue;
                   col--;
                   break;

                 case RIGHT:

                   if( col + 1 == blocks[0].size() ) continue;
                   col++;
                   break;
               }
               if( Point( col , row ) == target )
               {
                 targeted = true;
                 break;
               }

               if( blocks[row][col].value != Block::OBSTACLE &&
                   blocks[row][col].value != netIndex )
               {
                 const Point block( col , row );
                 const int   l  = manhattanDistance( block , source ) +
                                  manhattanDistance( block , target );
                 const int   lP = manhattanDistance( sources.front() , source ) +
                                  manhattanDistance( sources.front() , target );
                 const int   d  = ( l   - MD ) / 2;
                 const int   dP = ( lP  - MD ) / 2;

                 blocks[row][col].value = netIndex;

                 if( d > detour || d > dP )
                 {
                   nextDetour.push( block );
                   blocks[row][col].detour = detour + 1;
                 }
                 else
                 {
                   sources.push( block );
                   blocks[row][col].detour = detour;
                 }
               }
            }
            if( targeted ) break;

            sources.pop();
          }
          if( targeted ) break;

          swap( sources , nextDetour );
       }
       while( !sources.empty()     ) sources.pop();
       while( !nextDetour.empty()  ) nextDetour.pop();
       // end find path

       debug << "netnum : " << netIndex << " net : " << node->name() << endl;
       for( unsigned int i = blocks.size() - 1 ; i >= 0 ; i-- )
       {
          for( unsigned int j = 0 ; j < blocks[i].size() ; j++ )
             switch( blocks[i][j].value )
             {
               case Block::SPACE:     debug << "  "; break;
               case Block::OBSTACLE:  debug << "█"; break;
               default:

                 debug << setw( 2 );
                 if( blocks[i][j].value == netIndex )  debug << blocks[i][j].detour;
                 else                                  debug << blocks[i][j].value;
                 break;
             }
          debug << endl;
          if( i == 0 ) break;
       }
       debug << endl;

       // back trace
       if( targeted )
       {
         cout << "trace\n";
         forward_list<Point> path( 1 , target );
         stack<Point>        pathDetail;

         int   row           = target.y();
         int   col           = target.x();
         int   currentDetour = detour + 1;
         auto  it            = path.begin();

         pathDetail.push( target );

         // set initial direction
         if(       blocks[row+1][col].value == netIndex )
         {
           direction = UP;
           row++;
         }
         else if(  blocks[row-1][col].value == netIndex )
         {
           direction = DOWN;
           row--;
         }
         else if(  blocks[row][col+1].value == netIndex )
         {
           direction = RIGHT;
           col++;
         }
         else
         {
           direction = LEFT;
           col--;
         }
         // end set initial direction

         while( Point( col , row ) != source )
         {
           int direct;
           int rowT;
           int colT;

           if      ( col ==  source.x() )
           {
             if( row > source.y() )        direct = DOWN;
             else                          direct = UP;
           }
           else if ( col >   source.x() )  direct = LEFT;
           else                            direct = RIGHT;

           for( int i = 0 ; i < DIRECT_NUM ; i++ , direct++ )
           {
              rowT = row;
              colT = col;

              if( i == 1 && col != source.x() )
              {
                if( row > source.y() ) direct = DOWN;
                else                   direct = UP;
              }

              if( direct == DIRECT_NUM ) direct = UP;

              switch( direct )
              {
                case UP:     rowT++; break;
                case DOWN:   rowT--; break;
                case LEFT:   colT--; break;
                case RIGHT:  colT++; break;
              }

              if( Point( colT , rowT ) == target ) continue;

              if(  blocks[rowT][colT].value  == netIndex &&
                   blocks[rowT][colT].detour <= currentDetour )
              {
                blocks[rowT][colT].value = Block::SPACE;
                pathDetail.push( Point( col , row ) );
                if( direction != direct )
                {
                  direction  = direct;
                  it         = path.insert_after( it , Point( col , row ) );
                }
                row            = rowT;
                col            = colT;
                currentDetour  = blocks[row][col].detour;
                break;
              }
           }
           if( row != rowT || col != colT )
           {
             row = pathDetail.top().y();
             col = pathDetail.top().x();
             pathDetail.pop();
           }
         }
         it = path.insert_after( it , source );

         Layer layer;
         auto  tail = path.begin();

         tail++;

         for( auto head = path.begin() ; tail != path.end() ; head++ , tail++ )
         {
            if      ( head->x() == tail->x() )
            {
              layer.setType ( Layer::METAL1 );
              layer.setPin  ( head->y() , tail->y() );
              layer.setTrack( head->x() );
            }
            else if ( head->y() == tail->y() )
            {
              layer.setType ( Layer::METAL2 );
              layer.setPin  ( head->x() , tail->x() );
              layer.setTrack( head->y() );
            }
            node->segments().push_back( layer );
         }

         for( Point &point : path )
            debug << point << " ";
         debug << endl;
       }
       // end back trace
     }
     source = target;
  }
}

void ICRouting::leeAlgorithm( Point &source , NetNode *node , int netIndex )
{
  // route from corner point
  enum Direction
  {
    UP,
    DOWN,
    LEFT,
    RIGHT
  };

  queue<Point> waveFront;
  int          waveFrontNum;
  int          waveIndex = 0;
  bool         finished  = false;
  int          direction;
  Point        target;

  waveFront.push( source );
  blocks[source.y()][source.x()].visit = netIndex;
  target = source;

  // Lee Algorithm
  while( !waveFront.empty() )
  {
    waveFrontNum = waveFront.size();

    for( int i = 0 ; i < waveFrontNum ; i++ )
    {
       for( int direct = UP ; direct <= RIGHT ; direct++ )
       {
          unsigned int row = waveFront.front().y();
          unsigned int col = waveFront.front().x();

          switch( direct )
          {
            case UP:

              if( row + 1 == blocks.size() ) continue;
              row++;
              if( blocks[row][col].value == Block::OBSTACLE ||
                  blocks[row][col].visit == netIndex ) continue;
              if( row + 1 == blocks.size() )
              {
                finished   = true;
                direction  = DOWN;
              }
              break;

            case DOWN:

              if( row == 0 ) continue;
              row--;
              if( blocks[row][col].value == Block::OBSTACLE ||
                  blocks[row][col].visit == netIndex ) continue;
              if( row == 0 )
              {
                finished   = true;
                direction  = UP;
              }
              break;

            case LEFT:

              if( col == 0 ) continue;
              col--;
              if( blocks[row][col].value == Block::OBSTACLE ||
                  blocks[row][col].visit == netIndex ) continue;
              if( col == 0 )
              {
                finished   = true;
                direction  = RIGHT;
              }
              break;

            case RIGHT:

              if( col + 1 == blocks[0].size() ) continue;
              col++;
              if( blocks[row][col].value == Block::OBSTACLE ||
                  blocks[row][col].visit == netIndex ) continue;
              if( col + 1 == blocks[0].size() )
              {
                finished   = true;
                direction  = LEFT;
              }
              break;
          }

          blocks[row][col].value  = netIndex;
          blocks[row][col].visit  = netIndex;
          blocks[row][col].detour = waveIndex;

          if( finished )
          {
            source.set( col , row );
            break;
          }
          waveFront.push( Point( col , row ) );
       }
       if( finished ) break;

       waveFront.pop();
    }
    if( finished ) break;
    waveIndex++;
  }
  while( !waveFront.empty() ) waveFront.pop();
  // end Lee Algorithm

  forward_list<Point>  path( 1 , source );
  unsigned int         row = source.y();
  unsigned int         col = source.x();
  auto                 it  = path.begin();

  // back trace
  for( int i = waveIndex - 1 ; i >= 0 ; i-- )
  {
     for( int direct = UP ; direct <= RIGHT ; direct++ )
     {
        unsigned int rowT = row;
        unsigned int colT = col;

        switch( direct )
        {
          case UP:

            if( rowT == blocks.size() - 1 ) continue;
            rowT++;
            break;

          case DOWN:

            if( rowT == 0 ) continue;
            rowT--;
            break;

          case LEFT:

            if( colT == 0 ) continue;
            colT--;
            break;

          case RIGHT:

            if( colT == blocks[0].size() - 1 ) continue;
            colT++;
            break;
        }
        if(  blocks[rowT][colT].visit == netIndex && blocks[rowT][colT].detour == i )
        {
          row = rowT;
          col = colT;
          if( direct != direction )
            it = path.insert_after( it , Point( col , row ) );
        }
     }
  }
  // end back trace

  if(  ( row + 1 == target.y() && direction != UP    ) ||
       ( row - 1 == target.y() && direction != DOWN  ) ||
       ( col + 1 == target.x() && direction != RIGHT ) ||
       ( col - 1 == target.x() && direction != LEFT  ) )
    it = path.insert_after( it , Point( col , row ) );
  it = path.insert_after( it , target );

  auto tail = path.begin();

  tail++;

  for( auto head = path.begin() ; tail != path.end() ; head++ , tail++ )
  {
     Layer layer;

     if( head->x() == tail->x() )
     {
       layer.setType ( Layer::METAL1 );
       layer.setPin  ( head->y() , tail->y() );
       layer.setTrack( head->x() );
     }
     else
     {
       layer.setType ( Layer::METAL2 );
       layer.setPin  ( head->x() , tail->x() );
       layer.setTrack( head->y() );
     }
     node->segments().push_back( layer );
  }
  // route from corner point
}

void ICRouting::setupBlockCrossNum( NetNode *node )
{
  // setup block cross num
  for( Layer &layer : node->segments() )
  {
     const int MAX_SIDE = max( layer.headPin() , layer.tailPin() );
     const int MIN_SIDE = min( layer.headPin() , layer.tailPin() );
     int       row;
     int       col;

     switch( layer.type() )
     {
       case Layer::METAL1:

         col = layer.track();
         for( row = MIN_SIDE ; row <= MAX_SIDE ; row++ )
            blocks[row][col].crossNetY.push_front( node );
         break;

       case Layer::METAL2:

         row = layer.track();
         for( col = MIN_SIDE ; col <= MAX_SIDE ; col++ )
            blocks[row][col].crossNetX.push_front( node );
         break;

       default: break;
     }
  }
  // end setup block cross num
}

void ICRouting::setupCrossNet( const Point &acceptCrossNum , queue<Point> &waitedBlocks )
{
  // setup initial blocks whose cross net num is too large
  for( unsigned int row = 0 ; row < blocks.size() ; row++ )
     for( unsigned int col = 0 ; col < blocks[0].size() ; col++ )
     {
        Block &block = blocks[row][col];

        block.crossNetX.sort();
        block.crossNetY.sort();
        block.crossNetX.unique();
        block.crossNetY.unique();

        if( block.crossNetX.size() > acceptCrossNum.x() ||
            block.crossNetY.size() > acceptCrossNum.y() )
          waitedBlocks.push( Point( col , row ) );
        if( block.value != Block::OBSTACLE )
        {
          block.channelX.resize( acceptCrossNum.x() , nullptr );
          block.channelY.resize( acceptCrossNum.y() , nullptr );
        }
     }
  // end setup initial blocks whose cross net num is too large
}

void ICRouting::moveNets( const Point &acceptCrossNum , std::queue<Point> &waitedBlocks )
{
  // move nets
  while( !waitedBlocks.empty() )
  {
    unsigned int row = waitedBlocks.front().y();
    unsigned int col = waitedBlocks.front().x();

    while( blocks[row][col].crossNetX.size() > acceptCrossNum.x() )
    {
      bool moved = false;

      for( NetNode *movedNet : blocks[row][col].crossNetX )
      {
         Layer        layer;
         unsigned int layerIndex = 0;
         unsigned int movedRow;

         // find layer to be moved
         for( const Layer &layer : movedNet->segments() )
         {
            if( layer.track() == row &&
                ( layer.headPin() - col ) * ( layer.tailPin() - col ) <= 0 )
              break;
            layerIndex++;
         }
         if( layerIndex == movedNet->segments().size() ) continue;
         layer = movedNet->segments()[layerIndex];
         // end find layer to be moved

         // move layer
         const int MIN_Col = min( layer.headPin() , layer.tailPin() );
         const int MAX_Col = max( layer.headPin() , layer.tailPin() );
         // move up

         for( movedRow = row + 1 ; movedRow < blocks.size() ; movedRow++ )
         {
            bool end = false;

            // test whether layer can't move up
            for( int col = MIN_Col ; col <= MAX_Col ; col++ )
               if( blocks[movedRow][col].value == Block::OBSTACLE )
               {
                 end = true;
                 break;
               }
            if( end ) break;
            // end test whether layer can't move up

            // test whether layer can't put on this column
            for( int col = MIN_Col ; col < MAX_Col ; col++ )
               if( blocks[movedRow][col].crossNetX.size() > acceptCrossNum.x() )
               {
                 end = true;
                 break;
               }
            if( end ) continue;
            // end test whether layer can't put on this column

            moved = true;
            break;
         }
         assert( movedRow < blocks.size() );
         // end move up
         // move down
         if( !moved && row != 0 )
         {
           for( movedRow = row - 1 ; movedRow >= 0 ; movedRow-- )
           {
              bool end = false;

              // test whether layer can't move down
              for( int col = MIN_Col ; col <= MAX_Col ; col++ )
                 if( blocks[movedRow][col].value == Block::OBSTACLE )
                 {
                   end = true;
                   break;
                 }
              if( end ) break;
              // end test whether layer can't move down

              // test whether layer can't put on this column
              for( int col = MIN_Col ; col < MAX_Col ; col++ )
                 if(  blocks[movedRow][col].crossNetX.size() > acceptCrossNum.x() )
                 {
                   end = true;
                   break;
                 }
              assert( !( end && movedRow == 0 ) );
              if( end ) continue;
              // end test whether layer can't put on this column

              moved = true;
              break;
            }
         }
         // end move down
         if( moved )
         {
           int  minRow;
           int  maxRow;
           bool shorten;

           // setup x cross net
           for( int col = MIN_Col ; col <= MAX_Col ; col++ )
           {
              blocks[row][col].crossNetX.remove( movedNet );
              blocks[movedRow][col].crossNetX.push_back( movedNet );
           }
           movedNet->segments()[layerIndex].setTrack( movedRow );
           // end setup x cross net

           // setup y cross net
           layer    = movedNet->segments()[layerIndex+1];
           shorten  = ( ( layer.headPin() - movedRow ) *
                        ( layer.tailPin() - movedRow ) < 0 );
           if( shorten )
           {
             if( layer.headPin() > movedRow )
             {
               minRow = movedRow + 1;
               maxRow = layer.headPin();
             }
             else
             {
               minRow = layer.headPin();
               maxRow = movedRow - 1;
             }

             for( int row = minRow ; row <= maxRow ; row++ )
                blocks[row][layer.track()].crossNetY.remove( movedNet );
           }
           else
           {
             if( layer.headPin() > movedRow )
             {
               minRow = movedRow;
               maxRow = layer.headPin() - 1;
             }
             else
             {
               minRow = layer.headPin() + 1;
               maxRow = movedRow;
             }

             for( int row = minRow ; row <= maxRow ; row++ )
             {
                blocks[row][layer.track()].crossNetY.push_back( movedNet );
                if( blocks[row][layer.track()].crossNetY.size() >
                    acceptCrossNum.y() )
                  waitedBlocks.push( Point( layer.track() , row ) );
             }
           }
           movedNet->segments()[layerIndex+1].setHeadPin( movedRow );

           layer    = movedNet->segments()[layerIndex-1];
           shorten  = ( ( layer.headPin() - movedRow ) *
                        ( layer.tailPin() - movedRow ) < 0 );
           if( shorten )
           {
             if( layer.headPin() > movedRow )
             {
               minRow = movedRow + 1;
               maxRow = layer.headPin();
             }
             else
             {
               minRow = layer.headPin();
               maxRow = movedRow - 1;
             }

             for( int row = minRow ; row <= maxRow ; row++ )
                blocks[row][layer.track()].crossNetY.remove( movedNet );
           }
           else
           {
             if( layer.headPin() > movedRow )
             {
               minRow = movedRow;
               maxRow = layer.headPin() - 1;
             }
             else
             {
               minRow = layer.headPin() + 1;
               maxRow = movedRow;
             }

             for( int row = minRow ; row <= maxRow ; row++ )
             {
                blocks[row][layer.track()].crossNetY.push_back( movedNet );
                if( blocks[row][layer.track()].crossNetY.size() >
                    acceptCrossNum.y() )
                  waitedBlocks.push( Point( layer.track() , row ) );
             }
           }
           movedNet->segments()[layerIndex-1].setTailPin( movedRow );
           // end setup y cross net
           break;
         }
         // end move layer
      }
      if( !moved ) break;
      assert( moved );
    }
    while( blocks[row][col].crossNetY.size() > acceptCrossNum.y() )
    {
      bool moved = false;

      for( NetNode *movedNet : blocks[row][col].crossNetY )
      {
         Layer        layer;
         unsigned int layerIndex = 0;
         unsigned int movedCol;

         // find layer to be moved
         for( const Layer &layer : movedNet->segments() )
         {
            if( layer.track() == col &&
                ( layer.headPin() - row ) * ( layer.tailPin() - row ) <= 0 )
              break;
            layerIndex++;
         }
         if( layerIndex == movedNet->segments().size() ) continue;
         layer = movedNet->segments()[layerIndex];
         // end find layer to be moved

         // move layer
         const int MIN_Row = min( layer.headPin() , layer.tailPin() );
         const int MAX_Row = max( layer.headPin() , layer.tailPin() );
         // move right
         for( movedCol = col + 1 ; movedCol < blocks[0].size() ; movedCol++ )
         {
            bool end = false;

            // test whether layer can't move right
            for( int row = MIN_Row ; row <= MAX_Row ; row++ )
               if( blocks[row][movedCol].value == Block::OBSTACLE )
               {
                 end = true;
                 break;
               }
            if( end ) break;
            // end test whether layer can't move right

            // test whether layer can't put on this row
            for( int row = MIN_Row ; row < MAX_Row ; row++ )
               if( blocks[row][movedCol].crossNetY.size() > acceptCrossNum.y() )
               {
                 end = true;
                 break;
               }
            if( end ) continue;
            // end test whether layer can't put on this row

            moved = true;
            break;
         }
         // end move right
         // move left
         if( !moved && col != 0 )
         {
           for( movedCol = col - 1 ; movedCol >= 0 ; movedCol-- )
           {
              bool end = false;

              // test whether layer can't move left
              for( int row = MIN_Row ; row <= MAX_Row ; row++ )
                 if( blocks[row][movedCol].value == Block::OBSTACLE )
                 {
                   end = true;
                   break;
                 }
              if( end ) break;
              // end test whether layer can't move left

              // test whether layer can't put on this row
              for( int row = MIN_Row ; row < MAX_Row ; row++ )
                 if(  blocks[row][movedCol].crossNetY.size() > acceptCrossNum.y() )
                 {
                   end = true;
                   break;
                 }
              assert( !( end && movedCol == 0 ) );
              if( end ) continue;
              // end test whether layer can't put on this row

              moved = true;
              break;
           }
         }
         // end move left
         if( moved )
         {
           int  minCol;
           int  maxCol;
           bool shorten;

           // setup y cross net
           for( int row = MIN_Row ; row <= MAX_Row ; row++ )
           {
              blocks[row][col].crossNetY.remove( movedNet );
              blocks[row][movedCol].crossNetY.push_back( movedNet );
           }
           movedNet->segments()[layerIndex].setTrack( movedCol );
           // end setup y cross net

           // setup x cross net
           layer    = movedNet->segments()[layerIndex+1];
           shorten  = ( ( layer.headPin() - movedCol ) *
                        ( layer.tailPin() - movedCol ) < 0 );
           if( shorten )
           {
             if( layer.headPin() > movedCol )
             {
               minCol = movedCol + 1;
               maxCol = layer.headPin();
             }
             else
             {
               minCol = layer.headPin();
               maxCol = movedCol - 1;
             }

             for( int col = minCol ; col <= maxCol ; col++ )
                blocks[layer.track()][col].crossNetX.remove( movedNet );
           }
           else
           {
             if( layer.headPin() > movedCol )
             {
               minCol = movedCol;
               maxCol = layer.headPin() - 1;
             }
             else
             {
               minCol = layer.headPin() + 1;
               maxCol = movedCol;
             }

             for( int col = minCol ; col <= maxCol ; col++ )
             {
                blocks[layer.track()][col].crossNetX.push_back( movedNet );
                if( blocks[layer.track()][col].crossNetX.size() >
                    acceptCrossNum.x() )
                  waitedBlocks.push( Point( col , layer.track() ) );
             }
           }
           movedNet->segments()[layerIndex+1].setHeadPin( movedCol );

           if( layerIndex != 0 )
           {
             layer    = movedNet->segments()[layerIndex-1];
             shorten  = ( ( layer.headPin() - movedCol ) *
                          ( layer.tailPin() - movedCol ) < 0 );
             if( shorten )
             {
               if( layer.headPin() > movedCol )
               {
                 minCol = movedCol + 1;
                 maxCol = layer.headPin();
               }
               else
               {
                 minCol = layer.headPin();
                 maxCol = movedCol - 1;
               }

               for( int col = minCol ; col <= maxCol ; col++ )
                  blocks[layer.track()][col].crossNetX.remove( movedNet );
             }
             else
             {
               if( layer.headPin() > movedCol )
               {
                 minCol = movedCol;
                 maxCol = layer.headPin() - 1;
               }
               else
               {
                 minCol = layer.headPin() + 1;
                 maxCol = movedCol;
               }

               for( int col = minCol ; col <= maxCol ; col++ )
               {
                  blocks[layer.track()][col].crossNetX.push_back( movedNet );
                  if( blocks[layer.track()][col].crossNetX.size() > acceptCrossNum.x() )
                    waitedBlocks.push( Point( col , layer.track() ) );
               }
             }
             movedNet->segments()[layerIndex-1].setTailPin( movedCol );
           }
           // end setup y cross net
           break;
         }
         // end move layer
      }
      if( !moved ) break;
      assert( moved );
    }
    waitedBlocks.pop();
  }
  // end move nets
}

void ICRouting::setupNets( const double xUnit , const double yUnit )
{
  double metal2Width  = tech->rule( SpacingRule::MIN_WIDTH    , Layer::METAL2 );
  double metal1Width  = tech->rule( SpacingRule::MIN_WIDTH    , Layer::METAL1 );
  double via12Width   = tech->rule( SpacingRule::MIN_WIDTH    , Layer::VIA12  );

  int netIndex = 0;

  circuitModel->minRect().setCenter( 0 , 0 );

  for( NetNode *node : nets )
  {
     for( Layer &layer : node->segments() )
     {
        bool  connectIO = false;
        Point io;

        // test whether connect to io pin
        for( const Point &point : ioPos[netIndex] )
        {
           double halfH = circuitModel->height() / 2;
           double halfW = circuitModel->width () / 2;

           int ioRow = round( ( point.y() + halfH ) / rowUnit );
           int ioCol = round( ( point.x() + halfW ) / colUnit );

           if( ioRow == static_cast<int>( blocks.size   () ) - 1 ) ioRow--;
           if( ioCol == static_cast<int>( blocks[0].size() ) - 1 ) ioCol--;

           if(  ( layer.type() == Layer::METAL1 && layer.track() == ioCol &&
                  ( layer.headPin() == ioRow || layer.tailPin() == ioRow ) ) ||
                ( layer.type() == Layer::METAL2 && layer.track() == ioRow &&
                  ( layer.headPin() == ioCol || layer.tailPin() == ioCol ) )  )
           {
             connectIO  = true;
             io         = point;
             if(  ( layer.type() == Layer::METAL1 && layer.tailPin() == ioRow ) ||
                  ( layer.type() == Layer::METAL2 && layer.tailPin() == ioCol ) )
               layer.setPin( layer.tailPin() , layer.headPin() );
             break;
           }
        }
        // end test whether connect to io pin

        double x;
        double y;
        double height;
        double width;

        const int MIN_Block = min( layer.headPin() , layer.tailPin() );
        const int MAX_Block = max( layer.headPin() , layer.tailPin() );

        const double HALF_H = circuitModel->height() / 2;
        const double HALF_W = circuitModel->width () / 2;

        int row;
        int col;

        unsigned int      channelNum;
        vector<NetNode*>  *channel;

        if( connectIO )
        {
          switch( layer.type() )
          {
            case Layer::METAL1:

              col = layer.track();
              row = layer.tailPin();

              if( layer.tailPin() == 0 && node->type() == Node::IO )
              {
                y = circuitModel->minRect().bottom() ;
              }
              else if(  layer.tailPin() == blocks.size() - 1 &&
                        node->type() == Node::IO )
              {
                y = circuitModel->minRect().top();
              }
              else
              {
                channel = &blocks[row][col].channelX;

                for( channelNum = 0 ; channelNum < channel->size() ; channelNum++ )
                {
                   if( !(*channel)[channelNum]  )
                   {
                     (*channel)[channelNum] = node;
                     break;
                   }
                   if( (*channel)[channelNum] == node ) break;
                }
                assert( channelNum < channel->size() );

                y = row * rowUnit + channelNum * yUnit + metal2Width / 2 - HALF_H;
              }

              x       = io.x();
              y       = ( io.y() + y ) / 2;
              height  = abs( io.y() - y ) + metal2Width;
              width   = metal1Width;

              channelNum =
                static_cast<unsigned int>( floor(
                fmod( io.x() + HALF_W , colUnit ) / xUnit ) );

              for( row = MIN_Block ; row <= MAX_Block ; row++ )
              {
                 if( blocks[row][col].value == Block::OBSTACLE ) continue;
                 blocks[row][col].channelY[channelNum] = node;
              }
              break;

            case Layer::METAL2:

              col = layer.tailPin();
              row = layer.track();

              if( layer.tailPin() == 0 && node->type() == Node::IO )
              {
                x = circuitModel->minRect().left();
              }
              else if(  layer.tailPin() == blocks[0].size() - 1 &&
                        node->type() == Node::IO )
              {
                x = circuitModel->minRect().right();
              }
              else
              {
                channel = &blocks[row][col].channelY;

                for( channelNum = 0 ; channelNum < channel->size() ; channelNum++ )
                {
                   if( !(*channel)[channelNum] )
                   {
                     (*channel)[channelNum] = node;
                     break;
                   }
                   if( (*channel)[channelNum] == node ) break;
                }
                //assert( channelNum < channel->size() );

                x = col * colUnit + channelNum * xUnit + via12Width / 2 - HALF_W;
              }

              y       = io.y();
              height  = metal2Width;
              width   = abs( io.x() - x );
              x       = ( io.x() > x ) ? x + width / 2 : x - width / 2;
              width   += via12Width;

              channelNum =
                static_cast<unsigned int>( floor(
                fmod( io.y() + HALF_H , rowUnit ) / yUnit ) );

              for( col = MIN_Block ; col <= MAX_Block ; col++ )
              {
                 if( blocks[row][col].value == Block::OBSTACLE ) continue;
                 blocks[row][col].channelX[channelNum] = node;
              }
              break;

            default: break;
          }
        }
        else
        {
          double head;
          double tail;

          channelNum = 0;

          switch( layer.type() )
          {
            case Layer::METAL1:

              col = layer.track();

              for( row = MIN_Block ; row <= MAX_Block ; row++ )
              {
                 unsigned int channelNumT;

                 channel = &blocks[row][col].channelX;

                 for( channelNumT = 0 ; channelNumT < channel->size() ;
                      channelNumT++ )
                   if(  !(*channel)[channelNumT] ||
                        (*channel)[channelNumT] == node ) break;
                 //assert( channelNumT < channel->size() );
                 if( channelNumT > channelNum ) channelNum = channelNumT;
              }
              for( row = MIN_Block ; row <= MAX_Block ; row++ )
                 if( blocks[row][col].value != Block::OBSTACLE )
                 blocks[row][col].channelX[channelNum] = node;

              x = col * colUnit + channelNum * xUnit + via12Width / 2 - HALF_W;

              if( layer.headPin() == 0 && node->type() == Node::IO )
              {
                head = circuitModel->minRect().bottom();
              }
              else if(  layer.headPin() == blocks.size() - 1 &&
                        node->type() == Node::IO )
              {
                head = circuitModel->minRect().top();
              }
              else
              {
                row     = layer.headPin();
                channel = &blocks[row][col].channelY;

                for( channelNum = 0 ; channelNum < channel->size() ; channelNum++ )
                {
                   if( !(*channel)[channelNum] )
                   {
                     (*channel)[channelNum] = node;
                     break;
                   }
                   if( (*channel)[channelNum] == node ) break;
                }
                //assert( channelNum < channel->size() );

                head =  row * rowUnit + channelNum * yUnit + metal2Width / 2 -
                        HALF_H;
              }

              if( layer.tailPin() == 0 && node->type() == Node::IO )
              {
                tail = circuitModel->minRect().bottom();
              }
              else if(  layer.tailPin() == blocks.size() - 1 &&
                        node->type() == Node::IO )
              {
                tail = circuitModel->minRect().top();
              }
              else
              {
                row     = layer.tailPin();
                channel = &blocks[row][col].channelY;

                for( channelNum = 0 ; channelNum < channel->size() ; channelNum++ )
                {
                   if( !(*channel)[channelNum] )
                   {
                     (*channel)[channelNum] = node;
                     break;
                   }
                   if( (*channel)[channelNum] == node ) break;
                }
                //assert( channelNum < channel->size() );

                tail =  row * rowUnit + channelNum * yUnit + metal2Width / 2 -
                        HALF_H;
              }

              y       = ( head + tail ) / 2;
              height  = abs( head - tail ) + metal2Width;
              width   = metal1Width;
              break;

            case Layer::METAL2:

              row = layer.track();

              for( col = MIN_Block ; col <= MAX_Block ; col++ )
              {
                 unsigned int channelNumT;

                 channel = &blocks[row][col].channelY;

                 for( channelNumT = 0 ; channelNumT < channel->size() ;
                      channelNumT++ )
                   if(  !(*channel)[channelNumT] ||
                        (*channel)[channelNumT] == node ) break;
                 //assert( channelNumT < channel->size() );
                 if( channelNumT > channelNum ) channelNum = channelNumT;
              }
              for( col = MIN_Block ; col <= MAX_Block ; col++ )
                 if( blocks[row][col].value != Block::OBSTACLE )
                 blocks[row][col].channelY[channelNum] = node;

              y = row * rowUnit + channelNum * yUnit + metal2Width / 2 - HALF_H;

              if( layer.headPin() == 0 && node->type() == Node::IO )
              {
                head = circuitModel->minRect().left();
              }
              else if(  layer.headPin() == blocks[0].size() - 1 &&
                        node->type() == Node::IO )
              {
                head = circuitModel->minRect().right();
              }
              else
              {
                col     = layer.headPin();
                channel = &blocks[row][col].channelX;

                for( channelNum = 0 ; channelNum < channel->size() ; channelNum++ )
                {
                   if( !(*channel)[channelNum] )
                   {
                     (*channel)[channelNum] = node;
                     break;
                   }
                   if( (*channel)[channelNum] == node ) break;
                }
                //assert( channelNum < channel->size() );

                head =  col * colUnit + channelNum * xUnit + via12Width / 2 -
                        HALF_W;
              }

              if( layer.tailPin() == 0 && node->type() == Node::IO )
              {
                tail = circuitModel->minRect().left();
              }
              else if(  layer.tailPin() == blocks[0].size() - 1 &&
                        node->type() == Node::IO )
              {
                tail = circuitModel->minRect().right();
              }
              else
              {
                col     = layer.tailPin();
                channel = &blocks[row][col].channelX;

                for( channelNum = 0 ; channelNum < channel->size() ; channelNum++ )
                {
                   if( !(*channel)[channelNum] )
                   {
                     (*channel)[channelNum] = node;
                     break;
                   }
                   if( (*channel)[channelNum] == node ) break;
                }
                //assert( channelNum < channel->size() );

                tail =  col * colUnit + channelNum * xUnit + via12Width / 2 -
                        HALF_W;
              }

              x       = ( head + tail ) / 2;
              width   = abs( head - tail ) + via12Width;
              height  = metal2Width;
              break;

            default: break;
          }
        }
        layer.setCenter ( x , y );
        layer.setHeight ( height );
        layer.setWidth  ( width );
     }
     netIndex++;
  }
}
