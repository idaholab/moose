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
   _tempzero(_parameters.get<Real>("tempzero")),
   _tempmax(_parameters.get<Real>("tempmax")),
   _timeduration(_parameters.get<Real>("timeduration"))
  {}
  
protected:
  virtual Real computeQpResidual()
  {
  
    Real value;

    if(_t < _timeduration)
      value = _tempzero + _tempmax * std::sin((0.5/_timeduration) * libMesh::pi * _t);
    else
      value = _tempmax;
    
    return _u_face[_qp]- value;
  }

private:
  Real _tempzero;
  Real _tempmax;
  Real _timeduration;
};
 
#endif
