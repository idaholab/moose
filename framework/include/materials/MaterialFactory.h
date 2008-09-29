#include "Material.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>

#ifndef MATERIALFACTORY_H
#define MATERIALFACTORY_H

/**
 * Typedef to make things easier.
 */
typedef Material * (*MaterialBuildPtr)(Parameters parameters, unsigned int block_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);

/**
 * Typedef to make things easier.
 */
typedef Parameters (*MaterialParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename MaterialType>
Material * buildMaterial(Parameters parameters,
                         unsigned int block_id,
                         std::vector<std::string> coupled_to,
                         std::vector<std::string> coupled_as)
{
  return new MaterialType(parameters, block_id, coupled_to, coupled_as);
}

/**
 * Responsible for building Materials on demand and storing them for retrieval
 */
class MaterialFactory
{
public:
  static MaterialFactory * instance()
  {
    static MaterialFactory * instance;
    if(!instance)
      instance=new MaterialFactory;
    return instance;
  }

  template<typename MaterialType> 
  void registerMaterial(std::string name)
  {
    name_to_build_pointer[name]=&buildMaterial<MaterialType>;
    name_to_params_pointer[name]=&valid_params<MaterialType>;
  }

  void add(std::string name,
           Parameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to,
           std::vector<std::string> coupled_as)
  {
    active_materials[block_id] = (*name_to_build_pointer[name])(parameters,block_id,coupled_to,coupled_as);
  }

  Parameters getValidParams(std::string name)
  {
    if( name_to_params_pointer.find(name) == name_to_params_pointer.end() )
    {
      std::cerr<<std::endl<<"A _"<<name<<"_ is not registered Material "<<std::endl<<std::endl;
      error();
    }
    return name_to_params_pointer[name]();
  }

  Material * getMaterial(unsigned int block_id)
  {
    return active_materials[block_id];
  }

private:
  MaterialFactory(){}
  virtual ~MaterialFactory(){}

  std::map<std::string, MaterialBuildPtr> name_to_build_pointer;
  std::map<std::string, MaterialParamsPtr> name_to_params_pointer;

  std::map<int, Material *> active_materials;
};

#endif //MATERIALFACTORY_H
