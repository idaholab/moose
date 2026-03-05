//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSideVectorPostprocessor.h"
#include "SamplerBase.h"

class ADNumericalFlux3EqnBase;

/**
 * Computes internal fluxes for FlowChannel1Phase.
 */
class NumericalFlux3EqnInternalValues : public InternalSideVectorPostprocessor,
                                        protected SamplerBase
{
public:
  static InputParameters validParams();

  NumericalFlux3EqnInternalValues(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  using SamplerBase::threadJoin;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Area in current element
  const ADVariableValue & _A1;
  /// Area in neighbor element
  const ADVariableValue & _A2;

  /// Reconstructed rho*A values in current element
  const ADMaterialProperty<Real> & _rhoA1;
  /// Reconstructed rho*u*A values in current element
  const ADMaterialProperty<Real> & _rhouA1;
  /// Reconstructed rho*E*A values in current element
  const ADMaterialProperty<Real> & _rhoEA1;
  /// Reconstructed passive*A values in current element
  const ADMaterialProperty<std::vector<Real>> & _passives_times_area1;

  /// Reconstructed rho*A values in neighbor element
  const ADMaterialProperty<Real> & _rhoA2;
  /// Reconstructed rho*u*A values in neighbor element
  const ADMaterialProperty<Real> & _rhouA2;
  /// Reconstructed rho*E*A values in neighbor element
  const ADMaterialProperty<Real> & _rhoEA2;
  /// Reconstructed passive*A values in neighbor element
  const ADMaterialProperty<std::vector<Real>> & _passives_times_area2;

  /// Passive transport variable names
  const std::vector<VariableName> _passives_names;
  /// Number of passive transport variables
  const unsigned int _n_passives;

  /// Numerical flux user object
  const ADNumericalFlux3EqnBase & _numerical_flux;
};
