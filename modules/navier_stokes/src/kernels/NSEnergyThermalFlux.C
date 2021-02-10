//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NS.h"
#include "NSEnergyThermalFlux.h"

registerMooseObject("NavierStokesApp", NSEnergyThermalFlux);

InputParameters
NSEnergyThermalFlux::validParams()
{
  InputParameters params = NSKernel::validParams();
  params.addClassDescription("This class is responsible for computing residuals and Jacobian terms "
                             "for the k * grad(T) * grad(phi) term in the Navier-Stokes energy "
                             "equation.");
  params.addRequiredCoupledVar(NS::temperature, "temperature");
  return params;
}

NSEnergyThermalFlux::NSEnergyThermalFlux(const InputParameters & parameters)
  : NSKernel(parameters),
    _grad_temp(coupledGradient(NS::temperature)),
    _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
    // Temperature derivative computing object
    _temp_derivs(*this)
{
  // Store pointers to all variable gradients in a single vector.
  _gradU.resize(5);
  _gradU[0] = &_grad_rho;
  _gradU[1] = &_grad_rho_u;
  _gradU[2] = &_grad_rho_v;
  _gradU[3] = &_grad_rho_w;
  _gradU[4] = &_grad_rho_et;
}

Real
NSEnergyThermalFlux::computeQpResidual()
{
  // k * grad(T) * grad(phi)
  return _thermal_conductivity[_qp] * (_grad_temp[_qp] * _grad_test[_i][_qp]);
}

Real
NSEnergyThermalFlux::computeQpJacobian()
{
  // The "on-diagonal" Jacobian for the energy equation
  // corresponds to variable number 4.
  return computeJacobianHelper_value(/*var_number=*/4);
}

Real
NSEnergyThermalFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (isNSVariable(jvar))
    return computeJacobianHelper_value(mapVarNumber(jvar));
  else
    return 0.0;
}

Real
NSEnergyThermalFlux::computeJacobianHelper_value(unsigned var_number)
{
  // The value to return
  Real result = 0.0;

  // I used "ell" here as the loop counter since it matches the
  // "\ell" used in my LaTeX notes.
  for (unsigned int ell = 0; ell < 3; ++ell)
  {
    // Accumulate the first dot product term
    Real intermediate_result = _temp_derivs.get_grad(var_number) * _grad_phi[_j][_qp](ell);

    // Now accumulate the Hessian term
    Real hess_term = 0.0;
    for (unsigned n = 0; n < 5; ++n)
    {
      // hess_term += get_hess(m,n) * gradU[n](ell); // ideally... but you can't have a
      // vector<VariableGradient&> :-(
      hess_term += _temp_derivs.get_hess(var_number, n) *
                   (*_gradU[n])[_qp](ell); // dereference pointer to get value
    }

    // Accumulate the second dot product term
    intermediate_result += hess_term * _phi[_j][_qp];

    // Hit intermediate_result with the test function, accumulate in the final value
    result += intermediate_result * _grad_test[_i][_qp](ell);
  }

  // Return result, don't forget to multiply by "k"!
  return _thermal_conductivity[_qp] * result;
}
