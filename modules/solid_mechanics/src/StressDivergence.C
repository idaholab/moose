#include "StressDivergence.h"

#include "Material.h"

template<>
InputParameters validParams<StressDivergence>()
{
  InputParameters params;
  params.addRequiredParam<Real>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  return params;
}


StressDivergence::StressDivergence(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as)
  :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
   _component(parameters.get<Real>("component"))
{
  
}

void
StressDivergence::subdomainSetup()
{
  _stress = &_material->getTensorProperty("stress");
  _elasticity_tensor = &_material->getColumnMajorMatrixProperty("elasticity_tensor");
}

Real
StressDivergence::computeQpResidual()
{
  Real r = (*_stress)[_qp].row(_component) * _dtest[_i][_qp];
  
  return r;
}

Real
StressDivergence::computeQpJacobian()
{
  RealVectorValue value;
  for(unsigned int j = 0; j<LIBMESH_DIM; j++)
    for(unsigned int i = 0; i<LIBMESH_DIM; i++)
    {
      value(i) += 0.5*(*_elasticity_tensor)[_qp]( (LIBMESH_DIM*_component)+i,(LIBMESH_DIM*_component)+j) * _dphi[_j][_qp](j);
      value(i) += 0.5*(*_elasticity_tensor)[_qp]( _component+(i*LIBMESH_DIM),(LIBMESH_DIM*_component)+j) * _dphi[_j][_qp](j);
    }
  
  return value * _dtest[_i][_qp];
}
