//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations

/*
 * This kernel calculates the prefactor * grad_psi * grad_psi in Allen-Cahn equation for phase field
 * modeling of oxidation psi is the electric field variable. prefactor = 0.5 * d_permitivity/d_phi,
 * described in [Materials] using [DerivativeParsedMaterials]
 */

class MatGradSquareCoupled : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  MatGradSquareCoupled(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const VariableGradient & _grad_elec_potential;
  const unsigned int _elec_potential_var;

  const MaterialProperty<Real> & _prefactor;
  const MaterialProperty<Real> & _dprefactor_dphi;
  std::vector<const MaterialProperty<Real> *> _dprefactor_darg;
};
