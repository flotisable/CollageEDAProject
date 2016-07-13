#include "ICRouting.h"

#include <vector>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "../Lib/Model/SubcktModel.h"
#include "../Lib/Model/ICModel.h"
#include "../Lib/Model/MosModel.h"
#include "../Lib/Node/MosNode.h"
#include "../Lib/Node/NetNode.h"
#include "../Lib/Component/Mos.h"
#include "../Lib/Component/Layer.h"
#include "../Lib/Graphic/Rectangle.h"
#include "../Lib/TechFile/TechFile.h"

bool ICRouting::routing()
{
  vector<Model*>  &subcktModels = m_model->model()->subcktModel();
  bool            success     = true;

  for( int i = subcktModels.size() - 1 ; i >= 0 ; i-- )
  {
     SubcktModel* model = static_cast<SubcktModel*>( subcktModels[i] );

     if( model->model()->subcktCell().size() )
       success &= gridRouting   ( model );
     else
       success &= channelRouting( model );
  }

  return success &= gridRouting( m_model );
}

bool ICRouting::channelRouting( SubcktModel *model )
{
  vector<Node*> &mosNodes = model->model()->mosCell();

  // set number 設定編號
  sort( mosNodes.begin() , mosNodes.end() ,
        []( Node *front , Node *back )
        {
          if( front->center().y() == back->center().y() )
            return front->center().x() < back->center().x();
          return front->center().y() > front->center().y();
        } );

  int nmosBias = -1;
  int pmosFirst = 0;
  int nmosFirst;

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

  vector<NetNode*>  nets;

  // set cost
  for( Node *node : model->model()->io() )
     if( node->type() != Node::VDD && node->type() != Node::VSS )
     {
       node->setCost( node->connect().size() );
       nets.push_back( static_cast<NetNode*>( node ) );
     }
     else
       node->setCost( -1 );

  for( Node *node : model->model()->net() )
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
             nets[i]->nets().push_back( net );
           }
           net.setCenter( index[j] , 0 );
           net.setWidth ( connectMos );
        }
     }
     // end set net segment
  }
  // end set HCG
  
  // set VCG
  const int           VCG = 2;
  vector<vector<int>> vcg;
  
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

  // rough routing
  vector<Rectangle>         intervals;
  vector<vector<Layer>> netInfo;
  
  netInfo.resize( nets.size() );
  
  // put intervals
  for( unsigned int i = 0 ; i < nets.size() ; i++ )
  {
     vector<Layer>  &netlist = nets[i]->nets();
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
  int           track;
  
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
     
     vector<Layer> &netlist = nets[netIndex]->nets();
     
     for( unsigned int j = listIndex ; j < netlist.size() ; j++ )
     {
        if( netlist[j].center().x() == netlist[j].center().y() )
        {
          // height means metal layer and direction , width means pin or track
          netInfo[netIndex].push_back(
          Layer( "METAL1" , -1 , segmentTrack , 1 , netlist[j].center().x() ) );
          netInfo[netIndex].push_back(
          Layer( "METAL1" , segmentTrack , track  , 1 , netlist[j].center().x() ) );
          listIndex++;
        }
        else if(  netlist[j].center().x() <= leftEdge &&
                  leftEdge    <= netlist[j].center().y() )
        {
          if( netInfo[netIndex].empty() )
            switch( static_cast<int>( netlist[j].width() ) )
            {
              case Mos::NMOS: netInfo[netIndex].push_back(
                              Layer( "METAL1" , segmentTrack , track ,
                                          1 , leftEdge ) ); break;
              case Mos::PMOS: netInfo[netIndex].push_back(
                              Layer( "METAL1" , -1 , segmentTrack    ,
                                          1 , leftEdge ) ); break;
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
            
            netInfo[netIndex].push_back( Layer( "METAL1" , topEdge , bottomEdge ,
                                                    1 , leftEdge ) );
          }
          netInfo[netIndex].push_back( Layer( "METAL2" , segment.center() ,
                                                  2 , segmentTrack ) );
          if( rightEdge >= netlist[j].center().y() )
          {
            switch( static_cast<int>( netlist[j].height() ) )
            {
              case Mos::NMOS:

                netInfo[netIndex].push_back( Layer( "METAL1" , segmentTrack , track ,
                                                        1 , rightEdge ) );
                break;
                
              case Mos::PMOS:

                netInfo[netIndex].push_back( Layer( "METAL1" , -1 , segmentTrack    ,
                                                        1 , rightEdge ) );
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
     if( nets[i]->nets().size() )
       netInfo[i].push_back(
       Layer( "METAL1" , -1 , track , 1 , nets[i]->nets().back().center().x() ) );

  for( unsigned int i = 0 ; i < netInfo.size() ; i++ )
     nets[i]->nets() = netInfo[i];

  // end rough routing

  // detail routing
  double  metal1Width   = tech->rule( SpacingRule::MIN_WIDTH    , "METAL1"  );
  double  metal1Space   = tech->rule( SpacingRule::MIN_SPACING  , "METAL1"  );
  double  metal2Width   = tech->rule( SpacingRule::MIN_WIDTH    , "METAL2"  );
  double  metal2Space   = tech->rule( SpacingRule::MIN_SPACING  , "METAL2"  );
  double  via12Width    = tech->rule( SpacingRule::MIN_WIDTH    , "VIA12"   );
  double  via12Space    = tech->rule( SpacingRule::MIN_SPACING  , "VIA12"   );
  double  nimpSpace     = tech->rule( SpacingRule::MIN_SPACING  , "NIMP"    );
  double  pimpSpace     = tech->rule( SpacingRule::MIN_SPACING  , "PIMP"    );
  double  conWidth      = tech->rule( SpacingRule::MIN_WIDTH    , "CONT"    );
  
  Mos     *pmos         = static_cast<MosNode*>( mosNodes[0] )->model();
  Mos     *nmos         = static_cast<MosNode*>( mosNodes[nmosBias] )
                          ->model();
  
  double  xBias         = ( ( pmosFirst > nmosFirst ) ? mosNodes[nmosBias]
                                                        ->center().x() :
                                                        mosNodes[0]
                                                        ->center().x()     ) -
                          ( via12Width + via12Space ) + model->center().x();
  double  yBias         = mosNodes[0]->center().y() +
                          pmos->source()[Mos::METAL].bottom() -
                          metal1Space - metal2Width / 2 + model->center().y();
  double  mosCrossWidth = max(  pmos->implant().width() + pimpSpace ,
                                nmos->implant().width() + nimpSpace ) -
                          3 * ( via12Width + via12Space );

  for( NetNode *node : nets )
  {
     vector<Layer>  &netlist    = node->nets();
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
        bool      connectGate = static_cast<int>( netlist[j].width() ) % 3 ==
                                MosNode::G;
        double    xFix        = ( connectPmos ) ? 1 : 0;
        double    yFix        = ( connectNmos ) ? 1 : 0;

        switch( static_cast<int>( netlist[j].height() ) )
        {
          case 1:
          
            height  =   ( netlist[j].center().y() - netlist[j].center().x() -
                        xFix - yFix ) * ( metal2Width + metal2Space ) +
                        metal2Width;
            if( height < 0 ) height = 0;
            width   =   metal1Width;
            x       +=  netlist[j].width() * ( via12Width + via12Space ) +
                        floor( netlist[j].width() / 3 ) * mosCrossWidth;
            y       -=  ( netlist[j].center().x() + xFix  ) *
                        ( metal2Width + metal2Space ) +
                        ( height - metal2Width ) / 2;
            layer.setCenterX( x );
            if( connectPmos && connectNmos )
            {
              if( connectGate )
              {
                layer.setLayer    ( "CONT" );
                layer.setWidth   ( conWidth  );
                layer.setHeight  ( conWidth  );
                layer.setCenterY ( yBias     );
                netlist.push_back( layer );
                layer.setCenterY ( mosNodes[nmosBias]->center().y() +
                                  nmos->source()[Mos::METAL].top() +
                                  model->center().y() + metal1Space +
                                  metal2Width / 2 );
                netlist.push_back( layer );
                layer.setLayer    ( "METAL1" );
                layer.setWidth   ( metal1Width );
                layer.setHeight  ( mosNodes[0]->center().y() +
                                  pmos->source()[Mos::METAL].bottom() -
                                  metal1Space -
                                  ( mosNodes[nmosBias]->center().y() +
                                  nmos->source()[Mos::METAL].top() +
                                  metal1Space ) );
                layer.setCenterY ( mosNodes[0]->center().y() +
                                  pmos->source()[Mos::METAL].bottom() -
                                  metal1Space - layer.height() / 2 +
                                  model->center().y() );
              }
              else
              {
                layer.setLayer( "METAL1" );
                layer.setWidth   ( metal1Width );
                layer.setHeight  ( mosNodes[0]->center().y() +
                                  pmos->source()[Mos::METAL].bottom() -
                                  mosNodes[nmosBias]->center().y() -
                                  nmos->source()[Mos::METAL].top() );
                layer.setCenterY ( mosNodes[0]->center().y() +
                                  pmos->source()[Mos::METAL].bottom() -
                                  layer.height() / 2 + model->center().y() );
              }
              netlist.push_back( layer );
              break;
            }
            if( connectPmos )
            {
              if( connectGate )
              {
                layer.setLayer( "CONT" );
                layer.setWidth   ( conWidth  );
                layer.setHeight  ( conWidth  );
                layer.setCenterY ( yBias     );
              }
              else
              {
                layer.setLayer( "METAL1" );
                layer.setWidth   ( metal1Width );
                layer.setHeight  ( metal1Space );
                layer.setCenterY ( yBias + metal2Width / 2 +
                                  layer.height() / 2 );
              }
              netlist.push_back( layer );
            }
            if( connectNmos )
            {
              if( connectGate )
              {
                layer.setLayer( "CONT" );
                layer.setWidth   ( conWidth  );
                layer.setHeight  ( conWidth  );
                layer.setCenterY ( mosNodes[nmosBias]->center().y() +
                                  nmos->source()[Mos::METAL].top() +
                                  model->center().y() + metal1Space +
                                  metal2Width / 2 );
                netlist.push_back( layer );
                layer.setLayer( "METAL1" );
                layer.setWidth   ( metal1Width );
                layer.setHeight  ( metal1Width );
              }
              else
              {
                layer.setLayer( "METAL1" );
                layer.setWidth   ( metal1Width );
                layer.setHeight  ( yBias - ( track - 1 ) *
                                  ( metal2Width + metal2Space ) -
                                  metal2Width / 2 -
                                  ( mosNodes[nmosBias]->center().y() +
                                  nmos->source()[Mos::METAL].top() +
                                  model->center().y() ) );
                layer.setCenterY ( yBias - ( track - 1 ) *
                                  ( metal2Width + metal2Space ) -
                                  metal2Width / 2 - layer.height() / 2 );
              }
              netlist.push_back( layer );
            }
            break;
          
          case 2:
          
            height  =   metal2Width;
            width   =   ( netlist[j].center().y() - netlist[j].center().x() )
                        * ( via12Width + via12Space ) + via12Width +
                        ( floor( netlist[j].center().y() / 3 ) -
                          floor( netlist[j].center().x() / 3 ) ) *
                          mosCrossWidth;
            x       +=  netlist[j].center().x() *
                        ( via12Width + via12Space ) +
                        ( width - via12Width ) / 2 +
                        floor( netlist[j].center().x() / 3 ) * mosCrossWidth;
            y       -=  netlist[j].width() * ( metal2Width + metal2Space );

            layer.setLayer( "VIA12" );
            layer.setWidth   ( via12Width );
            layer.setHeight  ( via12Width );
            layer.setCenterY ( y );
            layer.setCenterX ( xBias + netlist[j].center().x() *
                              ( via12Width + via12Space ) +
                              floor( netlist[j].center().x() / 3 ) *
                              mosCrossWidth );
            netlist.push_back( layer );
            layer.setCenterX ( xBias + netlist[j].center().y() *
                              ( via12Width + via12Space ) +
                              floor( netlist[j].center().y() / 3 ) *
                              mosCrossWidth );
            netlist.push_back( layer );
            break;
        }
        netlist[j].setCenter( x , y   );
        netlist[j].setHeight( height  );
        netlist[j].setWidth ( width   );
     }
  }
  // end detail routing

  return true;
}

bool ICRouting::gridRouting( SubcktModel *model )
{
  return true;
}
