#ifndef MATERIALFACTORY_H
#define MATERIALFACTORY_H

#include "Material.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>


/**
 * Typedef to make things easier.
 */
typedef Material * (*MaterialBuildPtr)(std::string name, InputParameters parameters, unsigned int block_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);

/**
 * Typedef to make things easier.
 */
typedef InputParameters (*MaterialParamsPtr)();

/**
 * Typedef to hide implementation details
 */
typedef std::map<int, Material *>::iterator MaterialIterator;

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator MaterialNamesIterator;

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename MaterialType>
Material * buildMaterial(std::string name,
                         InputParameters parameters,
                         unsigned int block_id,
                         std::vector<std::string> coupled_to,
                         std::vector<std::string> coupled_as)
{
  return new MaterialType(name, parameters, block_id, coupled_to, coupled_as);
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
    name_to_build_pointer[name]=&buildMaterial<MaterialType>;
    name_to_params_pointer[name]=&valid_params<MaterialType>;
  }

  void add(std::string mat_name,
           std::string name,
           InputParameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to=std::vector<std::string>(0),
           std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
  InputParameters getValidParams(std::string name);
  
  Material * getMaterial(THREAD_ID tid, unsigned int block_id);
  
  void updateMaterialDataState();

  MaterialIterator activeMaterialsBegin(THREAD_ID tid);
  MaterialIterator activeMaterialsEnd(THREAD_ID tid);

  MaterialNamesIterator registeredMaterialsBegin();
  MaterialNamesIterator registeredMaterialsEnd();

private:
  MaterialFactory();
  
  virtual ~MaterialFactory(){}

  std::map<std::string, MaterialBuildPtr> name_to_build_pointer;
  std::map<std::string, MaterialParamsPtr> name_to_params_pointer;

  std::vector<std::string> _registered_material_names;
  std::vector<std::map<int, Material *> > active_materials;
};

#endif //MATERIALFACTORY_H
