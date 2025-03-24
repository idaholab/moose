//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PressureBase.h"
#include "MooseTypes.h"

#include "MooseTypes.h"
#include "libmesh/quadrature_gauss.h"

class Function;

/**
 * Pressure applies a pressure on a given boundary in the direction defined by component
 */

template <bool is_ad>
class PressureTempl : public PressureParent<is_ad>
{
public:
  static InputParameters validParams();

  PressureTempl(const InputParameters & parameters);

protected:
  GenericReal<is_ad> computePressure() const override;

  ///@{ Pressure value constant factor, function factor, and postprocessor factor
  const Real _factor;
  const Function * const _function;
  const PostprocessorValue * const _postprocessor;
  ///@}

  /// _alpha Parameter for HHT time integration scheme
  const Real _alpha;

  usingTransientInterfaceMembers;
  using PressureParent<is_ad>::_qp;
  using PressureParent<is_ad>::_q_point;
};

typedef PressureTempl<false> Pressure;
typedef PressureTempl<true> ADPressure;
