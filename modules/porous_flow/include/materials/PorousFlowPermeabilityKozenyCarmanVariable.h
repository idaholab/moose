//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPermeabilityKozenyCarmanBase.h"

/**
 * Material designed to provide the permeability tensor which is calculated
 * from porosity using a form of the Kozeny-Carman equation (e.g. Oelkers
 * 1996: Reviews in Mineralogy v. 34, p. 131-192):
 * k = k_ijk * A * phi^n / (1 - phi)^m
 * where k_ijk is a tensor providing the anisotropy, phi is porosity,
 * n and m are positive scalar constants.
 * A is provided as an auxVariable.
 */
template <bool is_ad>
class PorousFlowPermeabilityKozenyCarmanVariableTempl
  : public PorousFlowPermeabilityKozenyCarmanBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowPermeabilityKozenyCarmanVariableTempl(const InputParameters & parameters);

protected:
  /// retrieve constant value for A computed in constructor
  Real computeA() const override;

  /// Multiplying factor in k = k_ijk * A * phi^n / (1 - phi)^m
  const VariableValue & _A;

  usingPorousFlowPermeabilityBaseMembers;
};

typedef PorousFlowPermeabilityKozenyCarmanVariableTempl<false>
    PorousFlowPermeabilityKozenyCarmanVariable;
typedef PorousFlowPermeabilityKozenyCarmanVariableTempl<true>
    ADPorousFlowPermeabilityKozenyCarmanVariable;
