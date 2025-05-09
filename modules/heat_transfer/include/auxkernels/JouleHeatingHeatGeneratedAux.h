//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Auxiliary kernel for computing the heat generated from Joule heating
 */

/**
 *  NOTE: Directly coupling an electrostatic potential will be deprecated in the near future
 *        (10/01/2025). For the deprecated method, this kernel calculates the heat
 *        source term corresponding to joule heating,
 *          Q = J * E = elec_cond * grad_phi * grad_phi
 *          - where phi is the electrical potential.
 */
class JouleHeatingHeatGeneratedAux : public AuxKernel
{
public:
  static InputParameters validParams();

  JouleHeatingHeatGeneratedAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Set to true when an electrostatic potential is provided
  const bool _supplied_potential;
  /// Gradient of the coupled potential
  const VariableGradient & _grad_elec;
  /// Holds the electric conductivity coefficient from the material system if non-AD
  const MaterialProperty<Real> * const _elec_cond;
  //// Holds the electric conductivity coefficient from the material system if AD
  const ADMaterialProperty<Real> * const _ad_elec_cond;
  /// The Joule heating residual provided as a material object
  const ADMaterialProperty<Real> & _heating_residual;
};
