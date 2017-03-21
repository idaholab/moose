/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MaterialDerivativeTestKernel.h"

template <>
InputParameters
validParams<MaterialDerivativeTestKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Class used for testing derivatives of a material property.");
  params.addRequiredParam<MaterialPropertyName>(
      "material_property", "Name of material property for which derivatives are to be tested.");
  params.addRequiredCoupledVar("args", "List of variables the material property depends on");

  return params;
}

MaterialDerivativeTestKernel::MaterialDerivativeTestKernel(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _n_vars(_coupled_moose_vars.size()),
    _p(getMaterialProperty<Real>("material_property")),
    _p_derivatives(_n_vars)
{
  for (unsigned int m = 0; m < _n_vars; ++m)
    _p_derivatives[m] =
        &getMaterialPropertyDerivative<Real>("material_property", _coupled_moose_vars[m]->name());
}

Real
MaterialDerivativeTestKernel::computeQpResidual()
{
  return _p[_qp] * _test[_i][_qp];
}

Real
MaterialDerivativeTestKernel::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
MaterialDerivativeTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable number corresponding to jvar
  const unsigned int cvar = mapJvarToCvar(jvar);

  return (*_p_derivatives[cvar])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
