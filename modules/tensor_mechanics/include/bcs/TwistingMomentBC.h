//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "ADIntegratedBC.h"

class Function;

// switch parent class depending on is_ad value
template <bool is_ad>
using TwistingMomentBCParent = typename std::conditional<is_ad, ADIntegratedBC, IntegratedBC>::type;

/**
 * Apply a bending or twisting moment (torque) to a boundary
 */
template <bool is_ad>
class TwistingMomentBCTempl : public TwistingMomentBCParent<is_ad>
{
public:
  static InputParameters validParams();

  TwistingMomentBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual();

  Real componentJacobian(unsigned int component);

  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  unsigned int _component;

  const Point _origin;
  const RealVectorValue _torque;

  const Function & _factor;
  const Real & _alpha;
  const PostprocessorValue & _pmi;

  const unsigned int _ndisp;
  std::vector<unsigned int> _dvars;

  const Point _dummy_point;

  using TwistingMomentBCParent<is_ad>::_i;
  using TwistingMomentBCParent<is_ad>::_t;
  using TwistingMomentBCParent<is_ad>::_dt;
  using TwistingMomentBCParent<is_ad>::_qp;
  using TwistingMomentBCParent<is_ad>::_test;
  using TwistingMomentBCParent<is_ad>::_q_point;
};

using TwistingMomentBC = TwistingMomentBCTempl<false>;
using ADTwistingMomentBC = TwistingMomentBCTempl<true>;
