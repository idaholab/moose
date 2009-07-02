#ifndef BCFACTORY_H
#define BCFACTORY_H

#include "BoundaryCondition.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>


/**
 * Typedef to make things easier.
 */
typedef BoundaryCondition * (*BCBuildPtr)(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);

/**
 * Typedef to make things easier.
 */
typedef Parameters (*BCParamsPtr)();

/**
 * Typedef to hide implementation details
 */
typedef std::vector<BoundaryCondition *>::iterator BCIterator;

/**
 * Typedef to hide implementation details
 */
typedef std::vector<std::string>::iterator BCNamesIterator;

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename BCType>
BoundaryCondition * buildBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as)
{
  return new BCType(name, parameters, var_name, boundary_id, coupled_to, coupled_as);
}

/**
 * Responsible for building BCs on demand and storing them for retrieval
 */
class BCFactory
{
public:
  static BCFactory * instance();
  
  template<typename BCType> 
  void registerBC(std::string name)
  {
    name_to_build_pointer[name]=&buildBC<BCType>;
    name_to_params_pointer[name]=&valid_params<BCType>;
  }

  void add(std::string bc_name,
           std::string name,
           Parameters parameters,
           std::string var_name,
           unsigned int boundary_id,
           std::vector<std::string> coupled_to=std::vector<std::string>(0),
           std::vector<std::string> coupled_as=std::vector<std::string>(0));
  
  Parameters getValidParams(std::string name);
  
  BCIterator activeBCsBegin(THREAD_ID tid, unsigned int boundary_id);
  BCIterator activeBCsEnd(THREAD_ID tid, unsigned int boundary_id);

  BCIterator activeNodalBCsBegin(THREAD_ID tid, unsigned int boundary_id);
  BCIterator activeNodalBCsEnd(THREAD_ID tid, unsigned int boundary_id);

  BCNamesIterator registeredBCsBegin();
  BCNamesIterator registeredBCsEnd();
  
private:
  BCFactory();

  virtual ~BCFactory();

  std::map<std::string, BCBuildPtr> name_to_build_pointer;
  std::map<std::string, BCParamsPtr> name_to_params_pointer;

  std::vector<std::string> _registered_bc_names;
  
  std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > > active_bcs;
  std::vector<std::map<unsigned int, std::vector<BoundaryCondition *> > > active_nodal_bcs;
};

#endif //BCFACTORY_H
