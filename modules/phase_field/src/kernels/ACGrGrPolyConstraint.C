/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACGrGrPolyConstraint.h"

template<>
InputParameters validParams<ACGrGrPolyConstraint>()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Grain-Boundary model poly crystaline pnalty-based constraint to enforce an order parameter sum of unity");
  params.addRequiredCoupledVar("v", "Array of coupled variable names");
  params.addParam<Real>("penalty", 1.0, "Magnitude of the penalty prefactor (higher means stricter constraint enforcement)");
  return params;
}

ACGrGrPolyConstraint::ACGrGrPolyConstraint(const InputParameters & parameters) :
    ACBulk<Real>(parameters),
    _ncrys(coupledComponents("v")),
    _vals(_ncrys),
    _vals_var(_ncrys),
    _penalty(getParam<Real>("penalty"))
{
  // Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _vals[i] = &coupledValue("v", i);
    _vals_var[i] = coupled("v", i);
  }
}

Real
ACGrGrPolyConstraint::computeDFDOP(PFFunctionType type)
{
  // Calculate either the residual or Jacobian of the grain growth free energy
  switch (type)
  {
    case Residual:
    {
      // Sum all order parameters minus one
      Real sum = -1.0 + _u[_qp];
      for (unsigned int i = 0; i < _ncrys; ++i)
        sum += (*_vals[i])[_qp];

      return 2.0 * _penalty * sum;
    }

    case Jacobian:
      return 2.0 * _penalty;

    default:
      mooseError("Invalid type passed in");
  }
}

Real
ACGrGrPolyConstraint::computeQpOffDiagJacobian(unsigned int jvar)
{
  return 2.0 * _penalty;
}
