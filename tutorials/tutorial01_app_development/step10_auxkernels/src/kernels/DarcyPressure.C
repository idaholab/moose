#include "DarcyPressure.h"

registerMooseObject("BabblerApp", DarcyPressure);

InputParameters
DarcyPressure::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription("Compute the diffusion term for Darcy pressure ($p$) equation: "
                             "$-\\nabla \\cdot \\frac{\\mathbf{K}}{\\mu} \\nabla p = 0$");
  return params;
}

DarcyPressure::DarcyPressure(const InputParameters & parameters)
  : ADKernelGrad(parameters),
    _permeability(getADMaterialProperty<Real>("permeability")),
    _viscosity(getADMaterialProperty<Real>("viscosity"))
{
}

ADRealVectorValue
DarcyPressure::precomputeQpResidual()
{
  return (_permeability[_qp] / _viscosity[_qp]) * _grad_u[_qp];
}
