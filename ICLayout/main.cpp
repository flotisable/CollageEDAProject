//#define CLASS_TEST

#ifndef CLASS_TEST

#include <iostream>
#include <time.h>
using namespace std;

#include "../Lib/Spice/Hspice.h"
#include "../Lib/Model/CircuitModel.h"
#include "../Lib/TechFile/CadenceTechFile.h"
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
     TechFile     *techFile = new CadenceTechFile;
     CircuitModel *model;
     Hspice       hspice( techFile );
     Layout       *layout   = new SkillLayout;
     double       start = clock();

     hspice.setID( Hspice::VDD   , "VDD"   );
     hspice.setID( Hspice::GND   , "VSS"   );
     hspice.setID( Hspice::PMOS  , "P_18"  );
     hspice.setID( Hspice::NMOS  , "N_18"  );

     if( hspice.read( "Full_adder.sp" ) )
     {
       cout << "Read\n";

       if( techFile->read( "../test.txt" ) )  cout << "Read Tech File\n";
       else                                   cout << "Tech File Error\n";
       
       techFile->write();

       model = hspice.model();
       model->generate();

       ICPlacement  placer( model , techFile );
       ICRouting    router( model , techFile );

       placer.placement ();
       router.routing   ();

       layout->setCenter( 0 , 0 );
       layout->drawCircuit( model );
       layout->drawRect( "NWELL" , static_cast<Rectangle>( *model ) );

       if( hspice.write() ) cout << "Write\n";
       else                 cout << "Write Error\n";
     }
     else
       cout << "Read Error\n";

     double runTime = ( clock() - start ) / CLOCKS_PER_SEC;

     totalTime += runTime;

     //cout << runTime << endl;
     
     delete layout;
     delete techFile;
  }
  
  runTime << "´ú" << TIMES << "¦¸¡G" << totalTime / TIMES << "¬í" << endl;

  cin.get();
  return 0;
}
#else

#include "class_test.h"

#endif
