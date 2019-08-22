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

/**
 * Enforce the equality of the chemical potentials in sublattices of the same phase
 * D. Schwen et al. https://doi.org/10.1016/j.commatsci.2021.110466
 *
 * \see SLKKSPhaseConcentration
 */
class SLKKSChemicalPotential : public DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>
{
public:
  static InputParameters validParams();

  SLKKSChemicalPotential(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual void initialSetup();

private:
  ///@{ coupled variable for the other sublattice concentration cs
  unsigned int _cs_var;
  VariableName _cs_name;
  ///@}

  ///@{ chemical potentials and their derivatives w.r.t. the two sublattice concentrations
  const MaterialProperty<Real> & _dFdu;
  const MaterialProperty<Real> & _dFdcs;
  const MaterialProperty<Real> & _d2Fdu2;
  const MaterialProperty<Real> & _d2Fdcsu;
  ///@}

  ///@{ sublattice site fractions
  const Real _a_u;
  const Real _a_cs;
  ///@}

  ///@{ free energy derivatives
  std::vector<const MaterialProperty<Real> *> _d2Fdudarg;
  std::vector<const MaterialProperty<Real> *> _d2Fdcsdarg;
  ///@}
};
