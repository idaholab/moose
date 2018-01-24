//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CHEMICALOUTFLOWBC_H
#define CHEMICALOUTFLOWBC_H

#include "IntegratedBC.h"

// Forward Declarations
class ChemicalOutFlowBC;

template <>
InputParameters validParams<ChemicalOutFlowBC>();

/**
 * Implements a simple constant VectorNeumann BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class ChemicalOutFlowBC : public IntegratedBC
{
public:
  ChemicalOutFlowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  /// Diffusivity
  const MaterialProperty<Real> & _diff;
  /// Porosity
  const MaterialProperty<Real> & _porosity;
};

#endif // CHEMICALOUTFLOWBC_H
