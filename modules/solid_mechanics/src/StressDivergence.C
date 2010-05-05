#include "StressDivergence.h"

#include "Material.h"

template<>
InputParameters validParams<StressDivergence>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<Real>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  return params;
}


StressDivergence::StressDivergence(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _component(parameters.get<Real>("component"))
{}

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
