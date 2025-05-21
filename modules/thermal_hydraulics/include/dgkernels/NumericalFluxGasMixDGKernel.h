//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADDGKernel.h"

class NumericalFluxGasMixBase;

/**
 * Adds side fluxes from NumericalFluxGasMix objects.
 */
class NumericalFluxGasMixDGKernel : public ADDGKernel
{
public:
  static InputParameters validParams();

  NumericalFluxGasMixDGKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  /**
   * Creates the mapping of coupled variable index to index in equation system
   *
   * @returns the mapping of coupled variable index to index in equation system
   */
  std::map<unsigned int, unsigned int> getIndexMapping() const;

  /// Area
  const ADVariableValue & _A_elem;
  const ADVariableValue & _A_neig;

  // reconstructed variable values in this cell
  const ADMaterialProperty<Real> & _xirhoA1;
  const ADMaterialProperty<Real> & _rhoA1;
  const ADMaterialProperty<Real> & _rhouA1;
  const ADMaterialProperty<Real> & _rhoEA1;

  // reconstructed variable values in neighbor cell
  const ADMaterialProperty<Real> & _xirhoA2;
  const ADMaterialProperty<Real> & _rhoA2;
  const ADMaterialProperty<Real> & _rhouA2;
  const ADMaterialProperty<Real> & _rhoEA2;

  /// Numerical flux user object
  const NumericalFluxGasMixBase & _numerical_flux;

  // coupled variable indices
  const unsigned int _xirhoA_var;
  const unsigned int _rhoA_var;
  const unsigned int _rhouA_var;
  const unsigned int _rhoEA_var;

  /// map of coupled variable index to equations variable index convention
  const std::map<unsigned int, unsigned int> _jmap;

  /// index within the equation system of the equation upon which this kernel acts
  const unsigned int _equation_index;
};
