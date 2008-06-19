#include "BoundaryCondition.h"

// System includes
#include <map>
#include <string>
#include <vector>

// LibMesh includes
#include <parameters.h>

#ifndef BCFACTORY_H
#define BCFACTORY_H

/**
 * Typedef to make things easier.
 */
typedef BoundaryCondition * (*BCBuildPtr)(Parameters parameters, EquationSystems * es, std::string var_name, unsigned int boundary_id);

/**
 * Templated build function used for generating function pointers to build classes on demand.
 */
template<typename BCType>
BoundaryCondition * buildBC(Parameters parameters, EquationSystems * es, std::string var_name, unsigned int boundary_id)
{
  return new BCType(parameters, es, var_name, boundary_id);
}

/**
 * Responsible for building BCs on demand and storing them for retrieval
 */
class BCFactory
{
public:
  static BCFactory * instance()
  {
    static BCFactory * instance;
    if(!instance)
      instance=new BCFactory;
    return instance;
  }

  template<typename BCType> 
  void registerBC(std::string name)
  {
    name_to_build_pointer[name]=&buildBC<BCType>;
  }

  void add(std::string name, Parameters parameters, EquationSystems * es, std::string var_name, unsigned int boundary_id)
  {
    active_bcs.push_back((*name_to_build_pointer[name])(parameters,es,var_name,boundary_id));
  }

  std::vector<BoundaryCondition *>::iterator activeBCsBegin(){ return active_bcs.begin(); };
  std::vector<BoundaryCondition *>::iterator activeBCsEnd(){ return active_bcs.end(); };

private:
  BCFactory(){}
  virtual ~BCFactory(){}

  std::map<std::string, BCBuildPtr> name_to_build_pointer;
  std::vector<BoundaryCondition *> active_bcs;
};

#endif //BCFACTORY_H
