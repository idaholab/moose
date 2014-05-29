#include "MatDivergenceBC.h"

template<>
InputParameters validParams<MatDivergenceBC>()
{
  InputParameters params = validParams<DivergenceBC>();
  params.addRequiredParam<std::string>("prop_name", "The name of the material property");

  return params;
}

MatDivergenceBC::MatDivergenceBC(const std::string & name, InputParameters parameters) :
    DivergenceBC(name, parameters),
    _mat(getMaterialProperty<Real>(getParam<std::string>("prop_name")))
{
}

MatDivergenceBC::~MatDivergenceBC()
{
}

Real
MatDivergenceBC::computeQpResidual()
{
  return _mat[_qp] * DivergenceBC::computeQpResidual();
}

Real
MatDivergenceBC::computeQpJacobian()
{
  return _mat[_qp] * DivergenceBC::computeQpJacobian();
}
