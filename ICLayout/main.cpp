//#define CLASS_TEST

#ifndef CLASS_TEST

#include <iostream>
#include <fstream>
#include <ctime>
using namespace std;

#include "../Lib/Spice/Hspice.h"
#include "../Lib/Model/CircuitModel.h"
#include "../Lib/TechFile/CadenceTechFile.h"
#include "../Lib/Layout/SkillLayout.h"
#include "../Lib/EDA/ICPlacement.h"
#include "../Lib/EDA/ICRouting.h"
#include "../Lib/Component/CircuitBoard.h"

int main()
{
  const int TIMES     = 1;
  double    totalTime = 0;
  fstream   runTime( "runTimeTest.txt" , ios::out );

  for( int i = 1 ; i <= TIMES ; i++ )
  {
     TechFile     *techFile = new CadenceTechFile;
     Hspice       hspice;
     Layout       *layout   = new SkillLayout;
     ICPlacement  placer;
     ICRouting    router;
     CircuitBoard circuitBoard( techFile , &hspice , &placer , &router ,
                                layout );
     double       start     = clock();

     hspice.setID( Hspice::VDD   , "VDD"   );
     hspice.setID( Hspice::GND   , "VSS"   );
     hspice.setID( Hspice::PMOS  , "P_18"  );
     hspice.setID( Hspice::NMOS  , "N_18"  );
     
     hspice.setCircuitBoard( &circuitBoard );

     if( hspice.read( "Full_adder.sp" ) )
     {
       cout << "Read\n";

       if( techFile->read( "../test.txt" ) )
       {
         cout << "Read Tech File\n";
         techFile->write();
       }
       else
       {
         cout << "Tech File Error\n";
         break;
       }

       circuitBoard.process();

       if( hspice.write() ) cout << "Write\n";
       else                 cout << "Write Error\n";
     }
     else
       cout << "Read Error\n";

     totalTime += ( clock() - start ) / CLOCKS_PER_SEC;
     
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
