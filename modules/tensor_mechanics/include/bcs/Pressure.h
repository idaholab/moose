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
#include "MooseTypes.h"

#include "MooseTypes.h"
#include "libmesh/quadrature_gauss.h"

class Function;

/**
 * Pressure applies a pressure on a given boundary in the direction defined by component
 */

template <bool is_ad>
using PressureParent = typename std::conditional<is_ad, ADIntegratedBC, IntegratedBC>::type;

template <bool is_ad>
class PressureTempl : public PressureParent<is_ad>
{
public:
  static InputParameters validParams();

  PressureTempl(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;
  virtual GenericReal<is_ad> computeQpResidual() override;

  GenericReal<is_ad> computeFactor() const;

  /// displacement component to apply the bc to
  unsigned int _component;

  /// Number of displacement variables
  const unsigned int _ndisp;

  ///@{ Pressure value constant factor, function factor, and postprocessor factor
  const Real _factor;
  const Function * const _function;
  const PostprocessorValue * const _postprocessor;
  ///@}

  /// _alpha Parameter for HHT time integration scheme
  const Real _alpha;

  /// Variable numbers of coupled displacement variables
  std::vector<unsigned int> _disp_var;

  const FEBase * const & _fe_side;
  const std::vector<RealGradient> * _q_dxi;
  const std::vector<RealGradient> * _q_deta;
  const std::vector<std::vector<Real>> * _phi_dxi;
  const std::vector<std::vector<Real>> * _phi_deta;
  const bool _use_displaced_mesh;

  // One FE for each thread
  std::vector<std::unique_ptr<FEBase>> _fe;

  std::map<unsigned int, unsigned int> _node_map;

  /// Coordinate system type
  Moose::CoordinateSystemType _coord_type;

  using PressureParent<is_ad>::_assembly;
  using PressureParent<is_ad>::_dt;
  using PressureParent<is_ad>::_fe_problem;
  using PressureParent<is_ad>::_i;
  using PressureParent<is_ad>::_mesh;
  using PressureParent<is_ad>::_name;
  using PressureParent<is_ad>::_normals;
  using PressureParent<is_ad>::_q_point;
  using PressureParent<is_ad>::_qp;
  using PressureParent<is_ad>::_sys;
  using PressureParent<is_ad>::_t;
  using PressureParent<is_ad>::_test;
  using PressureParent<is_ad>::_var;
};

class Pressure : public PressureTempl<false>
{
public:
  using PressureTempl<false>::PressureTempl;

protected:
  virtual Real computeQpJacobian() override;
  virtual void precalculateQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(const unsigned int jvar_num) override;
  virtual void precalculateQpOffDiagJacobian(const MooseVariableFEBase & jvar) override;

  Real computeStiffness(const unsigned int coupled_component);
  Real computeFaceStiffness(const unsigned int local_j, const unsigned int coupled_component);
};

typedef PressureTempl<true> ADPressure;
