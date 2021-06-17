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
using TorqueParent = typename std::conditional<is_ad, ADIntegratedBC, IntegratedBC>::type;

/**
 * Apply a torque as tractions distributed over a surface
 */
template <bool is_ad>
class TorqueTempl : public TorqueParent<is_ad>
{
public:
  static InputParameters validParams();

  TorqueTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual();

  Real componentJacobian(unsigned int component);

  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// coordinte axis this BC acts on
  unsigned int _component;

  /// pivot point for the torque
  const Point _origin;
  /// applied torque vector
  const RealVectorValue _torque;

  /// prefactor function (can only be time dependent)
  const Function & _factor;
  /// alpha parameter required for HHT time integration scheme
  const Real & _alpha;
  /// postprocessor that computes the polar moment of inertia
  const PostprocessorValue & _pmi;

  /// number of coupled displacement variables
  const unsigned int _ndisp;
  /// coupled displacement variables
  std::vector<unsigned int> _dvars;

  /// dummy point (zero) used in evaluating the time dependent prefactor
  const Point _dummy_point;

  using TorqueParent<is_ad>::_i;
  using TorqueParent<is_ad>::_t;
  using TorqueParent<is_ad>::_dt;
  using TorqueParent<is_ad>::_qp;
  using TorqueParent<is_ad>::_test;
  using TorqueParent<is_ad>::_q_point;
};

using Torque = TorqueTempl<false>;
using ADTorque = TorqueTempl<true>;
