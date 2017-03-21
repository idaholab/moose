/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMomentumViscousFlux.h"

template <>
InputParameters
validParams<NSMomentumViscousFlux>()
{
  InputParameters params = validParams<NSKernel>();
  params.addClassDescription(
      "Derived instance of the NSViscousFluxBase class for the momentum equations.");
  params.addRequiredParam<unsigned int>("component", "");
  return params;
}

NSMomentumViscousFlux::NSMomentumViscousFlux(const InputParameters & parameters)
  : NSKernel(parameters), _component(getParam<unsigned int>("component")), _vst_derivs(*this)
{
}

Real
NSMomentumViscousFlux::computeQpResidual()
{
  // Yay for less typing!
  const RealTensorValue & vst = _viscous_stress_tensor[_qp];

  // _component'th column of vst...
  RealVectorValue vec(vst(0, _component), vst(1, _component), vst(2, _component));

  // ... dotted with grad(phi), note: sign is positive as this term was -div(tau) on the lhs
  return vec * _grad_test[_i][_qp];
}

Real
NSMomentumViscousFlux::computeQpJacobian()
{
  Real value = 0.0;

  // Set variable names as in the notes
  const unsigned k = _component;
  const unsigned m = _component + 1; // _component = 0,1,2 -> m = 1,2,3 global variable number

  // Use external templated friend class for common viscous stress
  // tensor derivative computations.
  for (unsigned int ell = 0; ell < LIBMESH_DIM; ++ell)
    value += _vst_derivs.dtau(k, ell, m) * _grad_test[_i][_qp](ell);

  return value;
}

Real
NSMomentumViscousFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (isNSVariable(jvar))
  {
    Real value = 0.0;

    // Set variable names as in the notes
    const unsigned int k = _component;

    // Map jvar into the variable m for our problem, regardless of
    // how Moose has numbered things.
    unsigned int m = mapVarNumber(jvar);

    for (unsigned ell = 0; ell < LIBMESH_DIM; ++ell)
      value += _vst_derivs.dtau(k, ell, m) * _grad_test[_i][_qp](ell);

    return value;
  }
  else
    return 0.0;
}
