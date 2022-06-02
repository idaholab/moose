//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelValue.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"

/**
 * Enforce sum of phase sublattice concentrations to be the real concentration.
 * D. Schwen et al. https://doi.org/10.1016/j.commatsci.2021.110466
 *
 * \see SLKKSPhaseChemicalPotential
 */
class SLKKSMultiPhaseBase : public DerivativeMaterialInterface<JvarMapKernelInterface<KernelValue>>
{
public:
  static InputParameters validParams();

  SLKKSMultiPhaseBase(const InputParameters & parameters);

protected:
  ///@{ Sublattice concentrations
  const std::size_t _ncs;
  std::vector<const VariableValue *> _cs;
  const JvarMap & _cs_map;
  ///@}

  /// Number of sublattices per phase
  std::vector<unsigned int> _ns;

  ///@{ Order parameters for each phase \f$ \eta_j \f$
  const std::size_t _neta;
  std::vector<VariableName> _eta_names;
  const JvarMap & _eta_map;
  ///@}

  /// Sublattice site numbers
  std::vector<Real> _a_cs;

  ///@{ Switching function names
  std::vector<MaterialPropertyName> _h_names;
  const std::size_t _nh;
  ///@}

  /// phase index of each cs entry
  std::vector<unsigned int> _phase;

  /// Physical concentration
  const VariableValue & _c;
  const unsigned int _c_var;
};
