#include "libmesh_common.h"
#include "BoundaryCondition.h"

#ifndef SINDIRICHLETBC_H
#define SINDIRICHLETBC_H

//Forward Declarations
class SinDirichletBC;

template<>
Parameters valid_params<SinDirichletBC>();

class SinDirichletBC : public BoundaryCondition
{
public:

  SinDirichletBC(std::string name,
		  Parameters parameters, 
		  std::string var_name, unsigned int boundary_id, 
		  std::vector<std::string> coupled_to, 
		  std::vector<std::string> coupled_as)
  :BoundaryCondition(name, parameters, var_name, false, boundary_id, coupled_to, coupled_as),
   _initial(_parameters.get<Real>("initial")),
   _final(_parameters.get<Real>("final")),
   _duration(_parameters.get<Real>("duration"))
  {}
  
protected:
  virtual Real computeQpResidual()
  {
  
    Real value;

    if(_t < _duration)
      value = _initial + (_final-_initial) * std::sin((0.5/_duration) * libMesh::pi * _t);
    else
      value = _final;
    
    return _u_face[_qp]- value;
  }

private:
  Real _initial;
  Real _final;
  Real _duration;
};
 
#endif
