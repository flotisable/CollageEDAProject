#include "ICRouting.h"

#include <vector>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "../Model/CircuitModel.h"
#include "../Model/MosModel.h"
#include "../Node/MosNode.h"
#include "../Node/NetNode.h"
#include "../Component/Circuit.h"
#include "../Component/Mos.h"
#include "../Component/Layer.h"
#include "../Component/ViaDevice.h"
#include "../Graphic/Rectangle.h"
#include "../TechFile/TechFile.h"

bool ICRouting::routing( CircuitModel *model )
{
  if( model->circuitCell().size() ) return gridRouting    ( model );
  else                              return channelRouting ( model );
}

bool ICRouting::channelRouting( CircuitModel *model )
{
  circuitModel = model;

  channelCost   ();
  channelRough  ();
  channelDetail ();

  return true;
}

bool ICRouting::gridRouting( CircuitModel *model )
{
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

  nmosBias  = -1;
  pmosFirst = 0;

  for( unsigned int i = 0 ; i < mosNodes.size() ; i++ )
  {
     mosNodes[i]->setCost( i ); // use number as cost

     if(  nmosBias == -1 &&
          static_cast<MosNode*>( mosNodes[i] )->model()->Mos::type()
          == Mos::NMOS )
     {
       nmosBias = i;

       if( mosNodes[0]->center().x() <= mosNodes[i]->center().x() )
       {
         pmosFirst = 0;
         for( unsigned int j = 0 ; j < i ; j++ )
            if( mosNodes[j]->center().x() == mosNodes[i]->center().x() )
            {
              nmosFirst = j;
              break;
            }
       }
       else
       {
         nmosFirst = 0;
         for( unsigned int j = nmosBias + 1 ; j < mosNodes.size() ; j++ )
            if( mosNodes[j]->center().x() == mosNodes[0]->center().x() )
            {
              pmosFirst = j - nmosBias;
              break;
            }
       }
     }
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
  
  sort( nets.begin() , nets.end() , static_cast<bool (*)( Node* , Node* )>
        ( Node::costCompare ) );

  int maxPinNum = 0;

  // set HCG
  for( unsigned int i = 0 ; i < nets.size() ; i++ )
  {
     vector<Node*>  &mos = nets[i]->connect();
     Layer          net;

     nets[i]->setCost( i );
     net.setCenter( -1 , 0 );

     sort(  mos.begin() , mos.end() , []( Node* front , Node* back )
            {
              if( front->center().x() == back->center().x() )
                return front->center().y() > back->center().y();
              return front->center().x() < back->center().x();
            } );

     // set net segment
     for( Node *node : mos )
     {
        int   index[MosNode::PIN_NUM-1];
        int   pinIndex  = 0;
        int   bias;
        int   connectMos;

        if( node->cost() >= nmosBias && nmosBias != -1 )
        {
          bias        = -nmosBias + nmosFirst;
          connectMos  = Mos::NMOS;
        }
        else
        {
          bias        = pmosFirst;
          connectMos  = Mos::PMOS;
        }
        net.setHeight( connectMos );

        for( int j = MosNode::D ; j <= MosNode::S ; j++ )
           if( node->connect()[j] == nets[i] )
           {
             index[pinIndex] = ( node->cost() + bias ) * 3 + j;
             if( index[pinIndex] > maxPinNum ) maxPinNum = index[pinIndex];
             pinIndex++;
           }

        for( int j = 0 ; j < pinIndex ; j++ )
        {
           if( net.center().x() != -1  )
           {
             net.setCenter( net.center().x() , index[j] );
             nets[i]->segments().push_back( net );
           }
           net.setCenter( index[j] , 0 );
           net.setWidth ( connectMos );
        }
     }
     // end set net segment
  }
  // end set HCG

  // set VCG

  vcg.resize( maxPinNum + 1 );
  for( vector<int> &specVcg : vcg )
  {
     specVcg.resize( 2 , -1 );
     specVcg.push_back( 0 );
  }

  if( nmosBias != -1 )
  {
    int minPinNum = max(  pmosFirst , nmosFirst ) * 3;
    int vcgBias = minPinNum;

    for(  int i = minPinNum / 3 ; i <= maxPinNum / 3 ; i++ )
    {
       vector<Node*> nmosConnect = mosNodes[nmosBias+i]->connect();
       vector<Node*> pmosConnect = mosNodes[i]->connect();

       for( int j = MosNode::D ; j <= MosNode::S ; j++ )
       {
          if( nmosConnect[j]->cost() == pmosConnect[j]->cost() ) continue;
          vcg[vcgBias+j][Mos::NMOS] = nmosConnect[j]->cost();
          vcg[vcgBias+j][Mos::PMOS] = pmosConnect[j]->cost();
       }
       vcgBias += 3;
    }
  }
  // end set VCG
}

void ICRouting::channelRough()
{
  // rough routing
  vector<Rectangle>     intervals;
  vector<vector<Layer>> netInfo;

  netInfo.resize( nets.size() );

  // put intervals
  for( unsigned int i = 0 ; i < nets.size() ; i++ )
  {
     vector<Layer>  &netlist = nets[i]->segments();
     Layer          interval;

     interval.setWidth  ( i );
     interval.setHeight ( -1 );

     for( Rectangle &net : netlist )
     {
        for(  int k = net.center().x() ; k < net.center().y() ; k++ )
        {
           interval.setCenter( k , k + 1 );
           vcg[k  ][VCG]++;
           vcg[k+1][VCG]++;
           intervals.push_back( interval );
           netInfo[i].push_back( interval );
        }
        vcg[net.center().x()][VCG]--;
        vcg[net.center().y()][VCG]--;
     }
  }
  // end put intervals

  sort( intervals.begin() , intervals.end() ,
        []( const Rectangle &front , const Rectangle &back )
        {
          if( front.center().x() == back.center().x() )
            return front.width() < back.width();
          return front.center().x() < back.center().x();
        } );

  unsigned int  routeNum = 0;

  // unrestricted left-edge routing
  for( track = 0 ; routeNum < intervals.size() ; track++ )
  {
     int mark         = 0;
     int lastNetNum   = 0;
     int routeNumDiff = 0;

     for( unsigned int i = 0 ; i < intervals.size() ; i++ )
     {
        int leftEdge  = static_cast<int>( intervals[i].center().x() );
        int rightEdge = static_cast<int>( intervals[i].center().y() );
        int netNum    = static_cast<int>( intervals[i].width()      );

        if( rightEdge == -1 ) continue;
        if( leftEdge < mark ) continue;
        if( leftEdge == mark && mark != 0 && netNum != lastNetNum ) continue;

        // check vertical constraint
        bool  constraint = true;
        int   segmentNum = 1;
        int   rightEdgeT;

        if( vcg[leftEdge][Mos::NMOS] != -1 &&
            vcg[leftEdge][Mos::NMOS] == netNum && vcg[leftEdge][VCG] > 0 )
          continue;
        if( vcg[rightEdge][Mos::NMOS] != -1 &&
            vcg[rightEdge][Mos::NMOS] == netNum && vcg[rightEdge][VCG] > 0 )
          continue;
        if( vcg[leftEdge][Mos::PMOS] != -1 &&
            vcg[leftEdge][Mos::PMOS] != netNum )
          continue;
        if( vcg[rightEdge][Mos::PMOS] != -1 &&
            vcg[rightEdge][Mos::PMOS] != netNum )
        {
          for( unsigned int j = 0 ; j < netInfo[netNum].size() ; j++ )
             if( netInfo[netNum][j].center().x() == rightEdge )
             {
               for( unsigned int k = j ; k < netInfo[netNum].size() ; k++ )
               {
                  if( netInfo[netNum][k].height() != -1 ) break;
                  rightEdgeT = netInfo[netNum][k].center().y();

                  if( vcg[rightEdgeT][Mos::PMOS] != -1 &&
                      vcg[rightEdgeT][Mos::PMOS] != netNum )
                    continue;

                  constraint  =   false;
                  segmentNum  +=  k - j + 1;
                  break;
               }
               break;
             }
          if( constraint ) continue;
        }
        // end check vertical constraint

        mark        = ( constraint ) ? rightEdge : rightEdgeT;
        lastNetNum  = netNum;

        int index = i;

        for( int j = 0 ; j < segmentNum ; j++ )
        {
           intervals[index].setHeight( track );
           intervals[index].setCenter( leftEdge , -1 );
           routeNum++;
           routeNumDiff++;

           for( Rectangle &segment : netInfo[netNum] )
              if( segment.center().x() == leftEdge &&
                  segment.center().y() == rightEdge )
              {
                segment.setHeight( track );
                break;
              }

           vcg[leftEdge][VCG]--;
           vcg[rightEdge][VCG]--;

           if( vcg[leftEdge][Mos::PMOS] == netNum )
               vcg[leftEdge][Mos::PMOS] = -1;
           else if(  vcg[leftEdge][Mos::PMOS] == -1 )
           if(  netInfo[netNum][0].center().x() < leftEdge &&
                leftEdge <  netInfo[netNum][netInfo[netNum].size()-1]
                            .center().y() )
             for( Rectangle &segment : netInfo[netNum] )
                if( segment.center().y() == leftEdge &&
                    segment.height()     == -1 )
                {
                  vcg[leftEdge][Mos::PMOS] = netNum;
                  break;
                }

           if( constraint == false && j + 1 != segmentNum )
           {
             leftEdge++;
             rightEdge++;
             vcg[leftEdge][VCG]++;

             for( unsigned int k = index ; k < intervals.size() ; k++ )
                if( intervals[k].center().x() == leftEdge &&
                    intervals[k].width()      == netNum )
                {
                  index = k;
                  break;
                }
           }
        }

        if( vcg[rightEdge][Mos::PMOS] == netNum )
          vcg[rightEdge][Mos::PMOS] = -1;
        else if(  vcg[rightEdge][Mos::PMOS] == -1 )
          if( netInfo[netNum][0].center().x() < rightEdge &&
              rightEdge < netInfo[netNum][netInfo[netNum].size()-1]
                          .center().y() )
            for( Rectangle &segment : netInfo[netNum] )
               if(  segment.center().x() == rightEdge &&
                    segment.height()     == -1 )
               {
                 vcg[rightEdge][Mos::PMOS] = netNum;
                 break;
               }
     }
     if( routeNumDiff == 0 ) break;
  }
  // end unrestricted left-edge routing

  // merge segments
  sort( intervals.begin() , intervals.end() ,
        []( const Rectangle &front , const Rectangle &back )
        {
          if( front.width() == back.width() )
          {
            if( front.center().x() == back.center().x() )
              return front.height() < back.height();
            return front.center().x() < back.center().x();
          }
          return front.width() < back.width();
        } );

  int listIndex = 0;

  netInfo.clear();
  netInfo.resize( nets.size() );

  for( unsigned int i = 0 ; i < intervals.size() ; i++ )
  {
     Rectangle  segment       = intervals[i];
     int        leftEdge      = segment.center().x();
     int        rightEdge     = segment.center().x() + 1;
     int        segmentTrack  = segment.height();
     int        netIndex      = segment.width();

     for( unsigned int j = i + 1 ; j < intervals.size() ; j++ )
     {
        if( intervals[j].width  ()    == netIndex &&
            intervals[j].height ()    == segmentTrack &&
            intervals[j].center().x() == rightEdge )
          rightEdge++;
        else
        {
          i = j - 1;
          break;
        }
     }
     segment.setCenter( leftEdge , rightEdge );

     vector<Layer> &netlist = nets[netIndex]->segments();

     for( unsigned int j = listIndex ; j < netlist.size() ; j++ )
     {
        if( netlist[j].center().x() == netlist[j].center().y() )
        {
          // height means metal layer and direction , width means pin or track
          netInfo[netIndex].push_back( Layer( Layer::METAL1 ,
                                              -1 , segmentTrack ,
                                              netlist[j].center().x() ) );
          netInfo[netIndex].push_back( Layer( Layer::METAL1 ,
                                              segmentTrack  , track ,
                                              netlist[j].center().x() ) );
          listIndex++;
        }
        else if(  netlist[j].center().x() <= leftEdge &&
                  leftEdge    <= netlist[j].center().y() )
        {
          if( netInfo[netIndex].empty() )
            switch( static_cast<int>( netlist[j].width() ) )
            {
              case Mos::NMOS: netInfo[netIndex].push_back(
                              Layer(  Layer::METAL1 , segmentTrack , track ,
                                      leftEdge ) ); break;
              case Mos::PMOS: netInfo[netIndex].push_back(
                              Layer(  Layer::METAL1 , -1 , segmentTrack    ,
                                      leftEdge ) ); break;
            }
          else if( netInfo[netIndex].back().height() != 1 )
          {
            int topEdge;
            int bottomEdge;
            int lastTrack = netInfo[netIndex].back().width();

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

            netInfo[netIndex].push_back( Layer( Layer::METAL1  ,
                                                topEdge   , bottomEdge ,
                                                leftEdge ) );
          }
          netInfo[netIndex].push_back( Layer( Layer::METAL2 ,
                                              segment.center().x() ,
                                              segment.center().y() ,
                                              segmentTrack ) );
          if( rightEdge >= netlist[j].center().y() )
          {
            switch( static_cast<int>( netlist[j].height() ) )
            {
              case Mos::NMOS:

                netInfo[netIndex].push_back( Layer( Layer::METAL1 ,
                                                    segmentTrack , track ,
                                                    rightEdge ) );
                break;

              case Mos::PMOS:

                netInfo[netIndex].push_back( Layer( Layer::METAL1 ,
                                                    -1  , segmentTrack ,
                                                    rightEdge ) );
                break;
            }
            listIndex++;
          }
        }
     }

     if( i + 1 == intervals.size() || intervals[i+1].width() != netIndex )
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
              nets[i]->segments().back().center().x() ) );

  for( unsigned int i = 0 ; i < netInfo.size() ; i++ )
     nets[i]->segments() = netInfo[i];

  // end rough routing
}

void ICRouting::channelDetail()
{
  vector<Node*> &mosNodes = circuitModel->mosCell();

  // detail routing
  double  metal1Width   = tech->rule( SpacingRule::MIN_WIDTH    ,
                                      Layer::METAL1 );
  double  metal1Space   = tech->rule( SpacingRule::MIN_SPACING  ,
                                      Layer::METAL1 );
  double  metal2Width   = tech->rule( SpacingRule::MIN_WIDTH    ,
                                      Layer::METAL2 );
  double  metal2Space   = tech->rule( SpacingRule::MIN_SPACING  ,
                                      Layer::METAL2 );
  double  via12Width    = tech->rule( SpacingRule::MIN_WIDTH    ,
                                      Layer::VIA12  );
  double  via12Space    = tech->rule( SpacingRule::MIN_SPACING  ,
                                      Layer::VIA12  );
  double  nimpSpace     = tech->rule( SpacingRule::MIN_SPACING  ,
                                      Layer::NIMPLANT );
  double  pimpSpace     = tech->rule( SpacingRule::MIN_SPACING  ,
                                      Layer::PIMPLANT );
  double  conWidth      = tech->rule( SpacingRule::MIN_WIDTH    ,
                                      Layer::CONTACT );
  double  conInPoly     = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                      Layer::POLY1 , Layer::CONTACT );
  double  conInMetal    = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                      Layer::METAL1 , Layer::CONTACT );
  double  viaInMetal1   = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                      Layer::METAL1 , Layer::VIA12 );
  double  viaInMetal2   = tech->rule( SpacingRule::MIN_ENCLOSURE ,
                                      Layer::METAL2 , Layer::VIA12 );

  Mos     *pmos         = static_cast<MosNode*>( mosNodes[0] )->model();
  Mos     *nmos         = static_cast<MosNode*>( mosNodes[nmosBias] )
                          ->model();

  double  xBias         = ( ( pmosFirst > nmosFirst ) ? mosNodes[nmosBias]
                                                        ->center().x() :
                                                        mosNodes[0]
                                                        ->center().x()     ) -
                          ( via12Width + via12Space );
  double  yBias         = mosNodes[0]->center().y() +
                          pmos->source().upperLayer().bottom() -
                          metal1Space - metal2Width / 2;
  double  mosCrossWidth = max(  pmos->implant().width() + pimpSpace ,
                                nmos->implant().width() + nimpSpace ) -
                          3 * ( via12Width + via12Space );
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

  for( NetNode *node : nets )
  {
     vector<Layer>  &netlist    = node->segments();
     int            netlistSize = netlist.size();

     for( int j = 0 ; j < netlistSize ; j++ )
     {
        Layer     layer;
        double    x = xBias;
        double    y = yBias;
        double    height;
        double    width;
        bool      connectPmos = netlist[j].center().x() == -1;
        bool      connectNmos = netlist[j].center().y() == track;
        bool      connectGate = static_cast<int>( netlist[j].track() ) % 3 ==
                                MosNode::G;
        double    xFix        = ( connectPmos ) ? 1 : 0;
        double    yFix        = ( connectNmos ) ? 1 : 0;

        switch( netlist[j].type() )
        {
          case Layer::METAL1:

            height  =   ( netlist[j].tailPin() - netlist[j].headPin() -
                        xFix - yFix ) * ( metal2Width + metal2Space ) +
                        metal2Width;
            if( height < 0 ) height = 0;
            width   =   metal1Width;
            x       +=  netlist[j].track() * ( via12Width + via12Space ) +
                        floor( netlist[j].track() / 3 ) * mosCrossWidth;
            y       -=  ( netlist[j].headPin() + xFix  ) *
                        ( metal2Width + metal2Space ) +
                        ( height - metal2Width ) / 2;
            layer.setCenterX( x );
            if( connectPmos && connectNmos )
            {
              if( connectGate )
              {
                contact.setCenter( x , yBias );
                node->contacts().push_back( contact );
                contact.setCenter( x ,  mosNodes[nmosBias]->center().y() +
                                        nmos->source().upperLayer().top() +
                                        metal1Space + metal2Width / 2 );
                node->contacts().push_back( contact );

                layer.setType     ( Layer::METAL1 );
                layer.setWidth    ( metal1Width );
                layer.setHeight   ( mosNodes[0]->center().y() +
                                    pmos->source().upperLayer().bottom() -
                                    metal1Space -
                                    ( mosNodes[nmosBias]->center().y() +
                                      nmos->source().upperLayer().top() +
                                      metal1Space ) );
                layer.setCenterY  ( mosNodes[0]->center().y() +
                                    pmos->source().upperLayer().bottom() -
                                    metal1Space - layer.height() / 2 );
              }
              else
              {
                layer.setType     ( Layer::METAL1 );
                layer.setWidth    ( metal1Width );
                layer.setHeight   ( mosNodes[0]->center().y() +
                                    pmos->source().upperLayer().top() -
                                    mosNodes[nmosBias]->center().y() -
                                    nmos->source().upperLayer().bottom() );
                layer.setCenterY  ( mosNodes[0]->center().y() +
                                    pmos->source().upperLayer().top() -
                                    layer.height() / 2 );
              }
              netlist.push_back( layer );
              break;
            }
            if( connectPmos )
            {
              if( connectGate )
              {
                contact.setCenter( x , yBias );
                node->contacts().push_back( contact );
              }
              else
              {
                layer.setType   ( Layer::METAL1 );
                layer.setWidth  ( metal1Width );
                layer.setHeight ( metal1Space +
                                  pmos->source().upperLayer().height() );
                layer.setCenterY( yBias + metal2Width / 2 +
                                  layer.height() / 2 );
                netlist.push_back( layer );
              }
            }
            if( connectNmos )
            {
              if( connectGate )
              {
                contact.setCenter( x ,  mosNodes[nmosBias]->center().y() +
                                        nmos->source().upperLayer().top() +
                                        metal1Space + metal2Width / 2 );
                node->contacts().push_back( contact );
              }
              else
              {
                layer.setType   ( Layer::METAL1 );
                layer.setWidth  ( metal1Width );
                layer.setHeight ( yBias - ( track - 1 ) *
                                  ( metal2Width + metal2Space ) -
                                  metal2Width / 2 -
                                  ( mosNodes[nmosBias]->center().y() +
                                  nmos->source().upperLayer().top() ) +
                                  nmos->source().upperLayer().height() );
                layer.setCenterY( yBias - ( track - 1 ) *
                                  ( metal2Width + metal2Space ) -
                                  metal2Width / 2 - layer.height() / 2 );
                netlist.push_back( layer );
              }
            }
            break;

          case Layer::METAL2:

            height  =   metal2Width;
            width   =   ( netlist[j].tailPin() - netlist[j].headPin() )
                        * ( via12Width + via12Space ) + via12Width +
                        ( floor( netlist[j].tailPin() / 3 ) -
                          floor( netlist[j].headPin() / 3 ) ) *
                          mosCrossWidth;
            x       +=  netlist[j].headPin() *
                        ( via12Width + via12Space ) +
                        ( width - via12Width ) / 2 +
                        floor( netlist[j].headPin() / 3 ) * mosCrossWidth;
            y       -=  netlist[j].track() * ( metal2Width + metal2Space );

            via12.setCenter(  xBias + netlist[j].headPin() *
                              ( via12Width + via12Space ) +
                                floor( netlist[j].headPin() / 3 ) *
                                mosCrossWidth , y );
            node->contacts().push_back( via12 );
            via12.setCenter(  xBias + netlist[j].tailPin() *
                              ( via12Width + via12Space ) +
                              floor( netlist[j].tailPin() / 3 ) *
                              mosCrossWidth , y );
            node->contacts().push_back( via12 );
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
