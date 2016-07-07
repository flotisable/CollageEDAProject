//#define CLASS_TEST

#ifndef CLASS_TEST

#include <iostream>
#include <time.h>
using namespace std;

#include "../Lib/Spice/Hspice.h"
#include "../Lib/Model/ICModel.h"
#include "../Lib/Model/SubcktModel.h"
#include "../Lib/TechFile/TechFile.h"
#include "../Lib/Layout/SkillLayout.h"
#include "ICPlacement.h"
#include "ICRouting.h"
#include <fstream>

int main()
{
  const int TIMES     = 1;
  double    totalTime = 0;
  fstream   runTime( "runTimeTest.txt" , ios::out );

  //cout.precision( 20 );

  for( int i = 1 ; i <= TIMES ; i++ )
  {
     TechFile     techFile;
     SubcktModel  *model;
     Hspice       hspice( &techFile );
     SkillLayout  skill;
     double       start = clock();

     hspice.setID( Hspice::VDD   , "VDD"   );
     hspice.setID( Hspice::GND   , "VSS"   );
     hspice.setID( Hspice::PMOS  , "P_18"  );
     hspice.setID( Hspice::NMOS  , "N_18"  );

     if( hspice.read( "Full_adder.sp" ) )
     {
       cout << "Read\n";

       if( techFile.read( "../test.txt" ) ) cout << "Read Tech File\n";
       else                                 cout << "Tech File Error\n";

       model = hspice.model();
       model->model()->generate();
    
       ICPlacement  placer( model , &techFile );
       ICRouting    router( model , &techFile );

       placer.placement ();
       router.routing   ();

       skill.setCenter( 0 , 0 );
       skill.drawSubckt( model->model() );
       skill.drawRect( "NWELL" , static_cast<Rectangle>( *model ) );

       if( hspice.write() ) cout << "Write\n";
       else                 cout << "Write Error\n";
     }
     else
       cout << "Read Error\n";

     if( model->model() ) delete model->model();

     double runTime = ( clock() - start ) / CLOCKS_PER_SEC;

     totalTime += runTime;

     //cout << runTime << endl;
  }
  
  runTime << "´ú" << TIMES << "¦¸¡G" << totalTime / TIMES << "¬í" << endl;

  cin.get();
  return 0;
}
#else

#include "class_test.h"

#endif
