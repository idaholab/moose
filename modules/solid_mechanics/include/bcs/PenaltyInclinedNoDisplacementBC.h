//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"
#include "IntegratedBC.h"

class Function;

/**
 * Weakly enforce an inclined BC (u\dot n = 0) using a penalty method.
 */

template <bool is_ad>
using PenaltyInclinedNoDisplacementBCParent =
    typename std::conditional<is_ad, ADIntegratedBC, IntegratedBC>::type;

template <bool is_ad>
class PenaltyInclinedNoDisplacementBCTempl : public PenaltyInclinedNoDisplacementBCParent<is_ad>
{
public:
  static InputParameters validParams();

  PenaltyInclinedNoDisplacementBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  // An integer corresponding to the direction
  const unsigned int _component;

  /// Coupled displacement variables
  const unsigned int _ndisp;
  const std::vector<const GenericVariableValue<is_ad> *> _disp;

  /// Variable IDs of coupled displacement variables
  const std::vector<unsigned int> _disp_var;

  Real _penalty;

  using PenaltyInclinedNoDisplacementBCParent<is_ad>::_i;
  using PenaltyInclinedNoDisplacementBCParent<is_ad>::_normals;
  using PenaltyInclinedNoDisplacementBCParent<is_ad>::_qp;
  using PenaltyInclinedNoDisplacementBCParent<is_ad>::_test;
};

class PenaltyInclinedNoDisplacementBC : public PenaltyInclinedNoDisplacementBCTempl<false>
{
public:
  using PenaltyInclinedNoDisplacementBCTempl<false>::PenaltyInclinedNoDisplacementBCTempl;

protected:
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(const unsigned int jvar_num) override;
};

typedef PenaltyInclinedNoDisplacementBCTempl<true> ADPenaltyInclinedNoDisplacementBC;
