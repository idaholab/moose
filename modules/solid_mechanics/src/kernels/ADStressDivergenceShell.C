//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADStressDivergenceShell.h"

// MOOSE includes
#include "Assembly.h"
#include "Material.h"
#include "MooseVariable.h"
#include "SystemBase.h"
#include "RankTwoTensor.h"
#include "NonlinearSystem.h"
#include "MooseMesh.h"
#include "ArbitraryQuadrature.h"
#include "DenseMatrix.h"

#include "libmesh/quadrature.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("SolidMechanicsApp", ADStressDivergenceShell);

InputParameters
ADStressDivergenceShell::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Quasi-static stress divergence kernel for Shell element");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 5",
      "An integer corresponding to the degree of freedom "
      "this kernel acts on. (0 for disp_x, "
      "1 for disp_y, 2 for disp_z, 3 for rot_x, 4 for rot_y)");
  params.addRequiredParam<std::string>("through_thickness_order",
                                       "Quadrature order in out of plane direction");
  params.addParam<bool>(
      "large_strain", false, "Set to true to turn on finite strain calculations.");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

ADStressDivergenceShell::ADStressDivergenceShell(const InputParameters & parameters)
  : ADKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _large_strain(getParam<bool>("large_strain"))
{
  _t_qrule = std::make_unique<libMesh::QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("through_thickness_order")));
  _t_weights = _t_qrule->get_weights();

  _stress.resize(_t_weights.size());
  _stress_old.resize(_t_weights.size());
  _B_mat.resize(_t_weights.size());
  if (_large_strain)
    _B_nl.resize(_t_weights.size());
  _J_map.resize(_t_weights.size());

  for (unsigned int i = 0; i < _t_weights.size(); ++i)
  {
    _stress[i] = &getADMaterialProperty<RankTwoTensor>("stress_t_points_" + std::to_string(i));
    _stress_old[i] =
        &getMaterialPropertyOldByName<RankTwoTensor>("stress_t_points_" + std::to_string(i));
    _B_mat[i] = &getADMaterialProperty<DenseMatrix<Real>>("B_t_points_" + std::to_string(i));
    if (_large_strain)
      _B_nl[i] = &getADMaterialProperty<DenseMatrix<Real>>("B_nl_t_points_" + std::to_string(i));

    _J_map[i] = &getADMaterialProperty<Real>("J_mapping_t_points_" + std::to_string(i));
  }
}

ADReal
ADStressDivergenceShell::computeQpResidual()
{
  _q_weights = _qrule->get_weights();
  ADReal residual = 0.0;
  ADReal residual1 = 0.0;
  for (_qp_z = 0; _qp_z < _t_weights.size(); ++_qp_z)
  {
    residual1 = (*_stress[_qp_z])[_qp](0, 0) * (*_B_mat[_qp_z])[_qp](0, _i + _component * 4) +
                (*_stress[_qp_z])[_qp](1, 1) * (*_B_mat[_qp_z])[_qp](1, _i + _component * 4) +
                2.0 * (*_stress[_qp_z])[_qp](0, 1) * (*_B_mat[_qp_z])[_qp](2, _i + _component * 4) +
                2.0 * (*_stress[_qp_z])[_qp](0, 2) * (*_B_mat[_qp_z])[_qp](3, _i + _component * 4) +
                2.0 * (*_stress[_qp_z])[_qp](1, 2) * (*_B_mat[_qp_z])[_qp](4, _i + _component * 4);

    if (_large_strain)
      residual1 +=
          ((*_stress_old[_qp_z])[_qp](0, 0) * (*_B_nl[_qp_z])[_qp](0, _i + _component * 4) +
           (*_stress_old[_qp_z])[_qp](1, 1) * (*_B_nl[_qp_z])[_qp](1, _i + _component * 4) +
           2.0 * (*_stress_old[_qp_z])[_qp](0, 1) * (*_B_nl[_qp_z])[_qp](2, _i + _component * 4) +
           2.0 * (*_stress_old[_qp_z])[_qp](0, 2) * (*_B_nl[_qp_z])[_qp](3, _i + _component * 4) +
           2.0 * (*_stress_old[_qp_z])[_qp](1, 2) * (*_B_nl[_qp_z])[_qp](4, _i + _component * 4));

    residual += residual1 * (*_J_map[_qp_z])[_qp] * _q_weights[_qp] * _t_weights[_qp_z] /
                (_ad_JxW[_qp] * _ad_coord[_qp]);
  }
  return residual;
}
