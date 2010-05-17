#ifndef MATERIALFACTORY_H
#define MATERIALFACTORY_H

#include "Material.h"

// System includes
#include <map>
#include <string>
#include <vector>

// Moose includes
#include <InputParameters.h>


/**
 * Typedef to make things easier.
 */
typedef Material * (*MaterialBuildPtr)(std::string name, MooseSystem & moose_system, InputParameters parameters); // , unsigned int block_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*MaterialParamsPtr)();

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator MaterialNamesIterator;

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename MaterialType>
Material * buildMaterial(std::string name, MooseSystem & moose_system, InputParameters parameters)
{
  return new MaterialType(name, moose_system, parameters);
}

/**
 * Responsible for building Materials on demand and storing them for retrieval
 */
class MaterialFactory
{
public:
  static MaterialFactory * instance();
  
  template<typename MaterialType> 
  void registerMaterial(std::string name)
  {
    _name_to_build_pointer[name]=&buildMaterial<MaterialType>;
    _name_to_params_pointer[name]=&validParams<MaterialType>;
  }

  Material *create(std::string mat_name,
                   std::string name,
                   MooseSystem & moose_system,
                   InputParameters parameters)
  {
    return (*_name_to_build_pointer[mat_name])(name, moose_system, parameters);
  }
  
  InputParameters getValidParams(std::string name);
  
  MaterialNamesIterator registeredMaterialsBegin();
  MaterialNamesIterator registeredMaterialsEnd();

private:
  MaterialFactory();
  
  virtual ~MaterialFactory(){}

  std::map<std::string, MaterialBuildPtr> _name_to_build_pointer;
  std::map<std::string, MaterialParamsPtr> _name_to_params_pointer;

  std::vector<std::string> _registered_material_names;
};

#endif //MATERIALFACTORY_H
