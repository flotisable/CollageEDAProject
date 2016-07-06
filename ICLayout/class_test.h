#include <iostream>
using namespace std;

#include "../Lib/TechFile/TechFile.h"
#include "../Lib/Component/Layer.h"
#include "../Lib/Component/Mos.h"
#include "../Lib/Component/ViaDevice.h"

int main()
{
  TechFile  tech;
  Layer     layer;
  Mos       mos( Mos::NMOS , 1 , 0.18 , 1 , &tech );
  ViaDevice b;

  tech.read( "../test.txt" );

  layer.setLayer( "layer" );
  layer.setCenter( 0 , 1 );
  layer.setHeight( 2 );
  layer.setWidth( 3 );
  
  mos.generate();
  
  cout << layer << endl;
  cout << mos   << endl;
  
  cin.get();
  return 0;
}
