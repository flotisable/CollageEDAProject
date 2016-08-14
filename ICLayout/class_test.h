#include <iostream>
using namespace std;

#include "../Lib/Component/ViaDevice.h"

void outputDevice( const ViaDevice &device );

int main()
{
  Layer::read();

  ViaDevice device;
  
  device.setViaLayer( Layer::VIA12 );
  device.setCenter( 0 , 0 );
  device.setRow( 2 );
  device.setColumn( 3 );
  
  device.setLowerLayer( Layer::DIFFUSION );
  device.setImpLayer( Layer::NIMPLANT );
  device.setConWidth( 10 );
  device.setConSpace( 20 );
  device.setConInUpper( 30 , 40 );
  device.setConInLower( 50 , 60 );
  device.setLowerInImp( 70 , 80 );
  
  device.generate();
  outputDevice( device );
  
  device.setViaLayer( Layer::CONTACT );
  device.setLowerLayer( Layer::METAL1 );
  device.setImpLayer( Layer::NIMPLANT );
  device.generate();
  outputDevice( device );
  
  device.setLowerLayer( Layer::DIFFUSION );
  device.setImpLayer( Layer::PIMPLANT );
  device.generate();
  outputDevice( device );
  
  cin.get();
  return 0;
}

void outputDevice( const ViaDevice &device )
{
  cout << "via type : " << Layer::map( device.viaLayer() )  << endl;
  cout << "center   : " << device.center()                  << endl;
  cout << "row      : " << device.row()                     << endl;
  cout << "column   : " << device.column()                  << endl;

  cout << "contacts :\n";
  for( unsigned int i = 0 ; i < device.row() ; i++ )
     for( unsigned int j = 0 ; j < device.column() ; j++ )
        cout << device.contact()[i][j] << endl;

  cout << "upper layer  : " << device.upperLayer()  << endl;
  cout << "lower layer  : " << device.lowerLayer()  << endl;
  cout << "implant      : " << device.implant()     << endl;
  cout << endl;
}
