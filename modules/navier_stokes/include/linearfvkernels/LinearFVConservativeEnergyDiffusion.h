//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "LinearFVDiffusion.h"

/**
 * Diffusion of temperature while solving for conserved thermal energy E = rho * cp * T.
 */
class LinearFVConservativeEnergyDiffusion : public LinearFVDiffusion
{
public:
  static InputParameters validParams();
  LinearFVConservativeEnergyDiffusion(const InputParameters & params);

  virtual Real computeElemMatrixContribution() override;
  virtual Real computeNeighborMatrixContribution() override;
  virtual Real computeElemRightHandSideContribution() override;
  virtual Real computeNeighborRightHandSideContribution() override;
  virtual Real computeBoundaryMatrixContribution(const LinearFVBoundaryCondition & bc) override;

private:
  Real elemInverseRhoCp() const;
  Real neighborInverseRhoCp() const;
  Real singleSidedInverseRhoCp() const;

  /// Cell-centered rho * cp functor
  const Moose::Functor<Real> & _rho_cp;
};
