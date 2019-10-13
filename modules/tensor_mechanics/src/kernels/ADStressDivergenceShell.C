/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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

registerADMooseObject("TensorMechanicsApp", ADStressDivergenceShell);

defineADValidParams(
    ADStressDivergenceShell,
    ADKernel,
    params.addClassDescription("Quasi-static stress divergence kernel for Shell element");
    params.addRequiredParam<unsigned int>("component",
                                          "An integer corresponding to the direction "
                                          "the variable this kernel acts in. (0 for disp_x, "
                                          "1 for disp_y, 2 for disp_z, 3 for rot_x, 4 for rot_y)");
    params.addRequiredParam<std::string>("order", "Quadrature order in out of plane direction");
    params.addParam<bool>("large_strain",
                          false,
                          "Set to true to turn on finite strain calculations.");
    params.set<bool>("use_displaced_mesh") = true;);

template <ComputeStage compute_stage>
ADStressDivergenceShell<compute_stage>::ADStressDivergenceShell(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),
    _component(getParam<unsigned int>("component")),
    _large_strain(getParam<bool>("large_strain"))
{
  _t_qrule = libmesh_make_unique<QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("order")));
  _t_weights = _t_qrule->get_weights();

  _stress.resize(_t_weights.size());
  _stress_old.resize(_t_weights.size());
  _B_mat.resize(_t_weights.size());
  if (_large_strain)
    _B_NL.resize(_t_weights.size());
  _Jmap.resize(_t_weights.size());

  for (unsigned int i = 0; i < _t_weights.size(); ++i)
  {
    _stress[i] = &getADMaterialProperty<RankTwoTensor>("stress_t_points_" + std::to_string(i));
    _stress_old[i] =
        &getMaterialPropertyOldByName<RankTwoTensor>("stress_t_points_" + std::to_string(i));
    _B_mat[i] = &getADMaterialProperty<DenseMatrix<Real>>("B_matrix_t_points_" + std::to_string(i));
    if (_large_strain)
      _B_NL[i] =
          &getADMaterialProperty<DenseMatrix<Real>>("BNL_matrix_t_points_" + std::to_string(i));

    _Jmap[i] = &getADMaterialProperty<Real>("J_mapping_t_points_" + std::to_string(i));
  }
}

template <ComputeStage compute_stage>
ADReal
ADStressDivergenceShell<compute_stage>::computeQpResidual()
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
          ((*_stress_old[_qp_z])[_qp](0, 0) * (*_B_NL[_qp_z])[_qp](0, _i + _component * 4) +
           (*_stress_old[_qp_z])[_qp](1, 1) * (*_B_NL[_qp_z])[_qp](1, _i + _component * 4) +
           2.0 * (*_stress_old[_qp_z])[_qp](0, 1) * (*_B_NL[_qp_z])[_qp](2, _i + _component * 4) +
           2.0 * (*_stress_old[_qp_z])[_qp](0, 2) * (*_B_NL[_qp_z])[_qp](3, _i + _component * 4) +
           2.0 * (*_stress_old[_qp_z])[_qp](1, 2) * (*_B_NL[_qp_z])[_qp](4, _i + _component * 4));

    residual += residual1 * (*_Jmap[_qp_z])[_qp] * _q_weights[_qp] * _t_weights[_qp_z] /
                (_ad_JxW[_qp] * _ad_coord[_qp]);
  }
  return residual;
}
