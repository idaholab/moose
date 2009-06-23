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
typedef Material * (*MaterialBuildPtr)(std::string name, Parameters parameters, unsigned int block_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);

/**
 * Typedef to make things easier.
 */
typedef Parameters (*MaterialParamsPtr)();

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename MaterialType>
Material * buildMaterial(std::string name,
                         Parameters parameters,
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
           Parameters parameters,
           unsigned int block_id,
           std::vector<std::string> coupled_to=std::vector<std::string>(0),
           std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
  Parameters getValidParams(std::string name);
  
  Material * getMaterial(THREAD_ID tid, unsigned int block_id);
  
void updateMaterialDataState();

  std::map<int, Material *>::iterator activeMaterialsBegin(THREAD_ID tid);
  std::map<int, Material *>::iterator activeMaterialsEnd(THREAD_ID tid);

private:
  MaterialFactory();
  
  virtual ~MaterialFactory(){}

  std::map<std::string, MaterialBuildPtr> name_to_build_pointer;
  std::map<std::string, MaterialParamsPtr> name_to_params_pointer;

  std::vector<std::map<int, Material *> > active_materials;
};

#endif //MATERIALFACTORY_H
