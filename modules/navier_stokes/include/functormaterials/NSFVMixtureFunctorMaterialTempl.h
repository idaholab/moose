//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * This is the material class used to compute phase averaged properties of mixtures
 */
template <bool is_ad>
class NSFVMixtureFunctorMaterialTempl : public FunctorMaterial
{
public:
  static InputParameters validParams();

  NSFVMixtureFunctorMaterialTempl(const InputParameters & parameters);

protected:
  /// Vector of phase 1 properties
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _phase_1_properties;

  /// Vector of phase 2 properties
  std::vector<const Moose::Functor<GenericReal<is_ad>> *> _phase_2_properties;

  /// Vector of phase 1 properties names
  const std::vector<MooseFunctorName> _phase_1_names;

  /// Vector of phase 2 properties names
  const std::vector<MooseFunctorName> _phase_2_names;

  /// Vector of mixture properties names
  const std::vector<MooseFunctorName> _mixture_names;

  /// Phase 1 fraction
  const Moose::Functor<GenericReal<is_ad>> & _phase_1_fraction;

  /// Whether to bound the phase fraction between 0 and 1 to avoid outlandish properties
  const bool _limit_pf;
};

typedef NSFVMixtureFunctorMaterialTempl<false> WCNSLinearFVMixtureFunctorMaterial;
typedef NSFVMixtureFunctorMaterialTempl<true> NSFVMixtureFunctorMaterial;
