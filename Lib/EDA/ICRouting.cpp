#include "ICRouting.h"

#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <iomanip>
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

  return true;
}

bool ICRouting::gridRouting()
{
  gridCost  ();
  gridRough ();
  gridDetail();

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

  const int PMOS_BIAS = 0;
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

  nets.clear();

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
        net.setHeight ( CONNECT_MOS );

        // setup pins connect to mos and find max pin num
        for( int j = MosNode::D ; j <= MosNode::S ; j++ )
           if( node->connect()[j] == nets[i] )
           {
             index[pinIndex] = ( node->cost() + BIAS ) * PINS_PER_MOS + j;
             if( index[pinIndex] > MAX_PIN_NUM )
               MAX_PIN_NUM = index[pinIndex];
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
           net.setHeadPin ( index[j] );
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
  vcg.resize( MAX_PIN_NUM + 1 );
  for( vector<int> &specVcg : vcg ) // initialize vcg
     specVcg.resize( Mos::TYPE_NUM , Mos::UNKNOWN );

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
  static bool first = true;
  fstream debug;
  
  if( first )
  {
    debug.open( "channelRoughLog.txt" , ios::out );
    first = false;
  }
  else
    debug.open( "channelRoughLog.txt" , ios::app );

  debug << "Model : " << circuitModel->name() << endl;

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
        debug << "track : " << track << " num : " << i << endl;
        for( Layer &segment : intervals )
        {
           segment.setType( Layer::UNKNOWN );
           debug << segment << endl;
        }
        debug << endl;
        debug << "vcg\n";
        for( auto specVcg : vcg )
        {
           for( auto vcg : specVcg )
              debug << vcg << "\t";
           debug << endl;
        }
        debug << endl;
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

     vector<Layer> &netlist = nets[netIndex]->segments();

     // connect segments
     for( unsigned int j = listIndex ; j < netlist.size() ; j++ )
     {
        if( netlist[j].headPin() == netlist[j].tailPin() )
        {
          netInfo[netIndex].push_back(
            Layer(  Layer::METAL1 , -1 , segmentTrack ,
                    netlist[j].headPin() ) );
          netInfo[netIndex].push_back(
            Layer(  Layer::METAL1 , segmentTrack , track ,
                    netlist[j].headPin() ) );
          listIndex++;
        }
        else if(  netlist[j].headPin()  <= leftEdge &&
                  leftEdge              <= netlist[j].tailPin() )
        {
          if( leftEdge == netlist[j].headPin() )
            switch( static_cast<int>( netlist[j].width() ) )
            {
              case Mos::NMOS:
              
                netInfo[netIndex].push_back(
                  Layer( Layer::METAL1 , segmentTrack , track , leftEdge ) );
                break;

              case Mos::PMOS:

                netInfo[netIndex].push_back(
                  Layer( Layer::METAL1 , -1 , segmentTrack , leftEdge ) );
                break;
            }
          if( !netInfo[netIndex].empty() &&
              netInfo[netIndex].back().type() != Layer::METAL1 )
          {
            int topEdge;
            int bottomEdge;
            int lastTrack = netInfo[netIndex].back().track();

            if( lastTrack > segmentTrack )
            {
              topEdge     = segmentTrack;
              bottomEdge  = lastTrack;
            }
            else
            {
              topEdge     = lastTrack;
              bottomEdge  = segmentTrack;
            }

            netInfo[netIndex].push_back(
              Layer( Layer::METAL1  , topEdge , bottomEdge , leftEdge ) );
          }
          netInfo[netIndex].push_back(
            Layer(  Layer::METAL2     , segment.headPin() ,
                    segment.tailPin() , segmentTrack ) );
          if( rightEdge >= netlist[j].tailPin() )
          {
            switch( static_cast<int>( netlist[j].height() ) )
            {
              case Mos::NMOS:

                netInfo[netIndex].push_back(
                  Layer(  Layer::METAL1 , segmentTrack ,
                          track , rightEdge ) );
                break;

              case Mos::PMOS:

                netInfo[netIndex].push_back(
                  Layer( Layer::METAL1 , -1 , segmentTrack , rightEdge ) );
                break;
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

  const int PMOS_BIAS = 0;

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
        bool    connectPmos = netlist[j].headPin() == -1;
        bool    connectNmos = netlist[j].tailPin() == track;
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
  fstream     debug;
  
  if( first )
  {
    debug.open( "gridCostLog.txt" , ios::out );
    first = false;
  }
  else
    debug.open( "gridCostLog.txt" , ios::app );

  rowUnit = DBL_MAX;
  colUnit = DBL_MAX;
  
  for( Node *node : circuitModel->circuitCell() ) // find minimum channel
  {
     CircuitModel *circuitModel = static_cast<CircuitNode*>( node )->model();

     Point channel( ( circuitModel->width () -
                      circuitModel->minRect().width() ) / 2 ,
                    ( circuitModel->height() -
                      circuitModel->minRect().height() ) / 2 );

     if( rowUnit > channel.y() ) rowUnit = channel.y();
     if( colUnit > channel.x() ) colUnit = channel.x();
  }
  
  int row = ceil( circuitModel->height() / rowUnit );
  int col = ceil( circuitModel->width () / colUnit );

  // setup grid maze
  rowUnit = circuitModel->height() / row;
  colUnit = circuitModel->width () / col;
  blocks.resize( row );
  for( vector<Block> &row : blocks ) row.resize( col );
  
  for( int i = 0 ; i < row ; i++ )
     for( int j = 0 ; j < col ; j++ )
        blocks[i][j].value = SPACE;
  // end setup grid maze
  
  // setup obstacle
  for( Node *node : circuitModel->circuitCell() )
  {
     Rectangle minRect =  static_cast<CircuitNode*>( node )
                          ->model()->minRect();

     minRect.setCenter( node->center() );
  
     double hHalf = circuitModel->height() / 2;
     double wHalf = circuitModel->width () / 2;
  
     int yMin = static_cast<int>( round(  ( minRect.bottom() + hHalf ) /
                                          rowUnit ) );
     int yMax = static_cast<int>( round(  ( minRect.top   () + hHalf ) /
                                          rowUnit ) );
     int xMin = static_cast<int>( round(  ( minRect.left  () + wHalf ) /
                                          colUnit ) );
     int xMax = static_cast<int>( round(  ( minRect.right () + wHalf ) /
                                          colUnit ) );

     debug  << node->name() << " "
            << static_cast<CircuitNode*>( node )->model()->name() << endl;
     debug  << "y : " << yMin << " ~ " << yMax << endl;
     debug  << "x : " << xMin << " ~ " << xMax << endl;

     for( int i = yMin ; i <= yMax ; i++ )
        for( int j = xMin ; j <= xMax ; j++ )
           blocks[i][j].value = OBSTACLE;
  }
  // end setup obstacle
  
  debug << "Model : " << circuitModel->name() << endl;
  debug << "row = " << row << " unit = " << rowUnit << endl;
  debug << "col = " << col << " unit = " << colUnit << endl;

  for( int i = row - 1 ; i >= 0 ; i-- )
  {
     for( int j = 0 ; j < col ; j++ )
        debug << blocks[i][j].value;
     debug << endl;
  }
}

void ICRouting::gridRough()
{
  static bool first = true;
  fstream     debug;
  
  if( first )
  {
    debug.open( "gridRoughLog.txt" , ios::out );
    first = false;
  }
  else
    debug.open( "gridRoughLog.txt" , ios::app );

  // setup nets
  vector<NetNode*> nets;

  for( Node *node : circuitModel->io() )
     if( node->type() != Node::VDD && node->type() != Node::VSS )
       nets.push_back( static_cast<NetNode*>( node ) );

  for( Node *node : circuitModel->net() )
     nets.push_back( static_cast<NetNode*>( node ) );
  // end setup nets
  
  int netIndex = 0;

  for( NetNode *node : nets )
  {
     // setup io pin
     for( Node *connect : node->connect() )
     {
        CircuitNode   *circuitNode  = static_cast<CircuitNode*>( connect );
        CircuitModel  *model        = circuitNode->model();
        int           index   = connect->searchConnectNode( node->name() );
        NetNode       *net    = static_cast<NetNode*>( model->io()[index] );
        
        Layer *top    = nullptr;
        Layer *bottom = nullptr;
        Layer *left   = nullptr;
        Layer *right  = nullptr;
        
        // find four side
        for( Layer &segment : net->segments() )
        {
           if( !top     || top->top()       < segment.top() )
             top    = &segment;
           if( !bottom  || bottom->bottom() > segment.bottom() )
             bottom = &segment;
           if( !left    || left->left()     > segment.left() )
             left   = &segment;
           if( !right   || right->right()   < segment.right() )
             right  = &segment;
        }
        // end find four side
        
        vector<Node*> cells;
        bool          circuitCells = model->circuitCell().size();
        
        if( circuitCells )  cells = model->circuitCell();
        else                cells = model->mosCell();
        
        for( Node *node : cells )
        {
           Rectangle minRect;

           if( circuitCells ) minRect = static_cast<CircuitNode*>( node )
                                        ->model()->minRect();
           else               minRect = static_cast<MosNode*>( node )
                                        ->model()->implant();
           minRect.setCenter( node->center() );
           
           if(  top && top->top() < minRect.bottom() &&
                top->left () < minRect.right() &&
                top->right() > minRect.left () )
             top = nullptr;
           if(  bottom && bottom->bottom() > minRect.top() &&
                bottom->left  () < minRect.right() &&
                bottom->right () > minRect.left () )
             bottom = nullptr;
           if(  left && left->left() > minRect.right() &&
                left->top   () > minRect.bottom () &&
                left->bottom() < minRect.top    () )
             left = nullptr;
           if(  right && right->right() < minRect.left() &&
                right->top    () > minRect.bottom () &&
                right->bottom () < minRect.top    () )
             right = nullptr;
        }
        
        double  hHalf = circuitModel->height() / 2;
        double  wHalf = circuitModel->width () / 2;
        int     row;
        int     col;
        Layer   segment;
        
        if( top )
        {
          segment = *top;
          segment.setCenter( circuitNode->center() + top->center() );
          row = static_cast<int>( round( ( segment.top() + hHalf ) /
                                          rowUnit ) );
          col = static_cast<int>( round( ( segment.center().x()  + wHalf ) /
                                          colUnit ) );
          blocks[row][col].value      = netIndex;
          blocks[row][col].connectNet = net;
        }
        if( bottom )
        {
          segment = *bottom;
          segment.setCenter( circuitNode->center() + bottom->center() );
          row = static_cast<int>( round( ( segment.bottom() + hHalf ) /
                                          rowUnit ) );
          col = static_cast<int>( round( ( segment.center().x() + wHalf ) /
                                          colUnit ) );
          blocks[row][col].value      = netIndex;
          blocks[row][col].connectNet = net;
        }
        if( left )
        {
          segment = *left;
          segment.setCenter( circuitNode->center() + left->center() );
          row = static_cast<int>( round( ( segment.center().y() + hHalf ) /
                                          rowUnit ) );
          col = static_cast<int>( round( ( segment.left() + wHalf ) /
                                          colUnit ) );
          blocks[row][col].value      = netIndex;
          blocks[row][col].connectNet = net;
        }
        if( right )
        {
          segment = *right;
          segment.setCenter( circuitNode->center() + right->center() );
          row = static_cast<int>( round( ( segment.center().y() + hHalf ) /
                                          rowUnit ) );
          col = static_cast<int>( round( ( segment.right() + wHalf ) /
                                          colUnit ) );
          blocks[row][col].value      = netIndex;
          blocks[row][col].connectNet = net;
        }
        debug << "netnum : " << netIndex << " net : " << node->name() << endl;
        for( unsigned int i = blocks.size() - 1 ; i >= 0 ; i-- )
        {
           for( unsigned int j = 0 ; j < blocks[i].size() ; j++ )
              debug << setw( 2 ) << blocks[i][j].value;
           debug << endl;
           if( i == 0 ) break;
        }
        debug << endl;
     }
     debug << "netnum : " << netIndex << " net : " << node->name() << endl;
     for( unsigned int i = blocks.size() - 1 ; i >= 0 ; i-- )
     {
        for( unsigned int j = 0 ; j < blocks[i].size() ; j++ )
           debug << setw( 2 ) << blocks[i][j].value;
        debug << endl;
        if( i == 0 ) break;
     }
     debug << endl;
     // end setup io pin
     netIndex++;
  }
  
  debug << "Model : " << circuitModel->name() << endl;
  for( unsigned int i = blocks.size() - 1 ; i >= 0 ; i-- )
  {
     for( unsigned int j = 0 ; j < blocks[i].size() ; j++ )
        debug << setw( 2 ) << blocks[i][j].value;
     debug << endl;
     if( i == 0 ) break;
  }
  debug << endl;
}

void ICRouting::gridDetail()
{
}
