#include "MaterialFactory.h"
#include <iostream>

void MaterialFactory::updateMaterialDataState()
{
  std::map<int, Material *>::iterator it = active_materials.begin();
  std::map<int, Material *>::iterator it_end = active_materials.end();

  std::cerr << "\n\n\n\nUpdating Material Data State\n";
  for(;it!=it_end;++it) 
    it->second->updateDataState(); 
}
