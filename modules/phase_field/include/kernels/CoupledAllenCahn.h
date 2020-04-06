//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ACBulk.h"

/**
 * CoupledAllenCahn uses the Free Energy function and derivatives
 * provided by a DerivativeParsedMaterial to compute the residual
 * for the bulk part of the Allen-Cahn equation, where the variational
 * free energy derivative is taken w.r.t. a coupled variable.
 */
class CoupledAllenCahn : public ACBulk<Real>
{
public:
  static InputParameters validParams();

  CoupledAllenCahn(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // coupled variable name
  VariableName _v_name;

  const MaterialProperty<Real> & _dFdV;
  const MaterialProperty<Real> & _d2FdVdEta;

  std::vector<const MaterialProperty<Real> *> _d2FdVdarg;
};
