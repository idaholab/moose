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
using PressureBaseParent = typename std::conditional<is_ad, ADIntegratedBC, IntegratedBC>::type;

template <bool is_ad>
class PressureBaseTempl : public PressureBaseParent<is_ad>
{
public:
  static InputParameters validParams();
  static InputParameters actionParams();

  PressureBaseTempl(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual GenericReal<is_ad> computePressure() const = 0;

  virtual GenericReal<is_ad> computeQpResidual() override final;

  /// Variable numbers of coupled displacement variables
  std::vector<unsigned int> _disp_var;

  /// Number of displacement variables
  const unsigned int _ndisp;

  /// displacement component to apply the bc to
  const unsigned int _component;

  /// Coordinate system type
  Moose::CoordinateSystemType _coord_type;

  usingTransientInterfaceMembers;
  using PressureBaseParent<is_ad>::_assembly;
  using PressureBaseParent<is_ad>::_fe_problem;
  using PressureBaseParent<is_ad>::_i;
  using PressureBaseParent<is_ad>::_mesh;
  using PressureBaseParent<is_ad>::_name;
  using PressureBaseParent<is_ad>::_normals;
  using PressureBaseParent<is_ad>::_q_point;
  using PressureBaseParent<is_ad>::_qp;
  using PressureBaseParent<is_ad>::_sys;
  using PressureBaseParent<is_ad>::_test;
  using PressureBaseParent<is_ad>::_var;
};

class PressureBase : public PressureBaseTempl<false>
{
public:
  PressureBase(const InputParameters & parameters);

protected:
  virtual Real computeQpJacobian() override final;
  virtual void precalculateQpJacobian() override final;
  virtual Real computeQpOffDiagJacobian(const unsigned int jvar_num) override final;
  virtual void precalculateQpOffDiagJacobian(const MooseVariableFEBase & jvar) override final;

  Real computeStiffness(const unsigned int coupled_component);
  Real computeFaceStiffness(const unsigned int local_j, const unsigned int coupled_component);

  const std::vector<RealGradient> * _q_dxi;
  const std::vector<RealGradient> * _q_deta;
  const std::vector<std::vector<Real>> * _phi_dxi;
  const std::vector<std::vector<Real>> * _phi_deta;
  const bool _use_displaced_mesh;

  // One FE for each thread
  std::vector<std::unique_ptr<FEBase>> _fe;

  std::map<unsigned int, unsigned int> _node_map;
};

typedef PressureBaseTempl<true> ADPressureBase;

template <bool is_ad>
using PressureParent = typename std::conditional<is_ad, ADPressureBase, PressureBase>::type;
