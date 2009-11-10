#ifndef SINDIRICHLETBC_H
#define SINDIRICHLETBC_H

#include "libmesh_common.h"
#include "BoundaryCondition.h"


//Forward Declarations
class SinDirichletBC;

template<>
InputParameters valid_params<SinDirichletBC>();

class SinDirichletBC : public BoundaryCondition
{
public:

  SinDirichletBC(std::string name,
		  InputParameters parameters, 
		  std::string var_name, unsigned int boundary_id, 
		  std::vector<std::string> coupled_to, 
		  std::vector<std::string> coupled_as);
  
protected:
  virtual Real computeQpResidual();

private:
  Real _initial;
  Real _final;
  Real _duration;
};
 
#endif
