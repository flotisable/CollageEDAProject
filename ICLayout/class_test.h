#include <iostream>
using namespace std;

#include "../Lib/Component/Layer.h"

int main()
{
  Layer::read();
  //Layer::write();
  
  for( int i = 0 ; i < Layer::TYPE_NUM ; i++ )
     cout << Layer::map( static_cast<Layer::Type>( i ) ) << endl;
  
  cin.get();
  return 0;
}
