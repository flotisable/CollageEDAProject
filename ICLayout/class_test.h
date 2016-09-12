#include <iostream>
using namespace std;

#include "../Lib/Component/Layer.h"

int main()
{
  Layer::setTypesName( Layer::DIFFUSION , "DIFF"      );
  Layer::setTypesName( Layer::POLY1     , "POLY1"    );
  Layer::setTypesName( Layer::METAL1    , "METAL1"       );
  Layer::setTypesName( Layer::METAL2    , "METAL2"  );
  Layer::setTypesName( Layer::METAL3    , "METAL3"      );
  Layer::setTypesName( Layer::METAL4    , "METAL4"      );
  Layer::setTypesName( Layer::METAL5    , "METAL5"      );
  Layer::setTypesName( Layer::METAL6    , "METAL6"      );
  Layer::setTypesName( Layer::CONTACT   , "CONT"      );
  Layer::setTypesName( Layer::VIA12     , "VIA12"      );
  Layer::setTypesName( Layer::VIA23     , "VIA23"      );
  Layer::setTypesName( Layer::VIA34     , "VIA34"      );
  Layer::setTypesName( Layer::VIA45     , "VIA45"      );
  Layer::setTypesName( Layer::VIA56     , "VIA56"      );
  Layer::setTypesName( Layer::NIMPLANT  , "NIMP"      );
  Layer::setTypesName( Layer::PIMPLANT  , "PIMP"      );
  Layer::setTypesName( Layer::NWELL     , "NWELL"      );
  Layer::setTypesName( Layer::PWELL     , "PWELL"      );
  
  Layer::write();
  
  cin.get();
  return 0;
}
