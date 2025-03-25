//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 *  This kernel supplies the heat source term corresponding to joule heating (Q).
 *  The residual is provided by the 'ElectromagneticHeatingMaterial' object and
 *  can be structured in the following formulations:
 *
 *  Time domain formulation:
 *    Q = (conductivity * E) * E,
 *    - where E is the time domain electric field.
 *
 *  Frequency domain formulation:
 *    Q = 0.5 Re( conductivity * E * E^* ),
 *    - where E is the real component of the frequency domain electric field,
 *    - and E^* is the complex conjugate of the electric field.
 *
 *  In the case of the time domain formulation, the electric field can be
 *  defined using an electromagnetic solver or using the electrostatic
 *  approximation, where:
 *    E = gradient(V)
 *    - where V is the electrostatic potential
 */

/**
 *  NOTE: Directly coupling an electrostatic potential will be deprecated in the near future
 *        (10/01/2025). For the deprecated method, this kernel calculates the heat
 *        source term corresponding to joule heating,
 *          Q = J * E = elec_cond * grad_phi * grad_phi
 *          - where phi is the electrical potential.
 */
class ADJouleHeatingSource : public ADKernelValue
{
public:
  static InputParameters validParams();

  ADJouleHeatingSource(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

private:
  /// Set to true when an electrostatic potential is provided
  const bool _supplied_potential;
  /// Gradient of the coupled potential
  const ADVariableGradient & _grad_potential;
  /// Electric conductivity coefficient
  const ADMaterialProperty<Real> & _elec_cond;
  /// The Joule heating residual provided as a material object
  const ADMaterialProperty<Real> & _heating_residual;
};
