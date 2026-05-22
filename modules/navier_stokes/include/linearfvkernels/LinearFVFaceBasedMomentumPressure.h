//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVElementalKernel.h"
#include "MooseLinearVariableFV.h"
#include "ConservativeSharpInterfaceRhieChowMassFlux.h"

/**
 * Adds the reduced-pressure momentum predictor forcing reconstructed from face
 * snGrad/face-force data, matching the interFoam-style predictor contract.
 */
class LinearFVFaceBasedMomentumPressure : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();
  LinearFVFaceBasedMomentumPressure(const InputParameters & params);

  virtual Real computeMatrixContribution() override;

  virtual Real computeRightHandSideContribution() override;

protected:
  MooseLinearVariableFV<Real> & getPressureVariable(const std::string & vname);

  const unsigned int _index;
  MooseLinearVariableFV<Real> & _pressure_var;
  const ConservativeSharpInterfaceRhieChowMassFlux & _sharp_mass_flux_provider;
};
