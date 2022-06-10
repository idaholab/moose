//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StressDivergenceTensors.h"

// MOOSE includes
#include "ElasticityTensorTools.h"
#include "Material.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", StressDivergenceTensors);

InputParameters
StressDivergenceTensors::validParams()
{
  InputParameters params = JvarMapKernelInterface<ALEKernel>::validParams();
  params.addClassDescription("Stress divergence kernel for the Cartesian coordinate system");
  params.addRequiredRangeCheckedParam<unsigned int>("component",
                                                    "component < 3",
                                                    "An integer corresponding to the direction "
                                                    "the variable this kernel acts in. (0 for x, "
                                                    "1 for y, 2 for z)");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");

  // maybe this should be deprecated in favor of args
  params.addCoupledVar("temperature",
                       "The name of the temperature variable used in the "
                       "ComputeThermalExpansionEigenstrain.  (Not required for "
                       "simulations without temperature coupling.)");

  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names",
      "List of eigenstrains used in the strain calculation. Used for computing their derivatives "
      "for off-diagonal Jacobian terms.");
  params.addCoupledVar("out_of_plane_strain",
                       "The name of the out_of_plane_strain variable used in the "
                       "WeakPlaneStress kernel.");
  MooseEnum out_of_plane_direction("x y z", "z");
  params.addParam<MooseEnum>(
      "out_of_plane_direction",
      out_of_plane_direction,
      "The direction of the out_of_plane_strain variable used in the WeakPlaneStress kernel.");
  params.addParam<std::string>("base_name", "Material property base name");
  params.set<bool>("use_displaced_mesh") = false;
  params.addParam<bool>(
      "use_finite_deform_jacobian", false, "Jacobian for corotational finite strain");
  params.addParam<bool>("volumetric_locking_correction",
                        false,
                        "Set to false to turn off volumetric locking correction");
  return params;
}

StressDivergenceTensors::StressDivergenceTensors(const InputParameters & parameters)
  : JvarMapKernelInterface<ALEKernel>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _use_finite_deform_jacobian(getParam<bool>("use_finite_deform_jacobian")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),
    _Jacobian_mult(getMaterialPropertyByName<RankFourTensor>(_base_name + "Jacobian_mult")),
    _component(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp),
    _out_of_plane_strain_coupled(isCoupled("out_of_plane_strain")),
    _out_of_plane_strain(_out_of_plane_strain_coupled ? &coupledValue("out_of_plane_strain")
                                                      : nullptr),
    _out_of_plane_strain_var(_out_of_plane_strain_coupled ? coupled("out_of_plane_strain") : 0),
    _out_of_plane_direction(getParam<MooseEnum>("out_of_plane_direction")),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh")),
    _avg_grad_test(_test.size(), std::vector<Real>(3, 0.0)),
    _avg_grad_phi(_phi.size(), std::vector<Real>(3, 0.0)),
    _volumetric_locking_correction(getParam<bool>("volumetric_locking_correction"))
{
  // get coupled displacements
  for (unsigned int i = 0; i < _ndisp; ++i)
    _disp_var[i] = coupled("displacements", i);

  // fetch eigenstrain derivatives
  const auto nvar = _coupled_moose_vars.size();
  _deigenstrain_dargs.resize(nvar);
  for (std::size_t i = 0; i < nvar; ++i)
    for (auto eigenstrain_name : getParam<std::vector<MaterialPropertyName>>("eigenstrain_names"))
      _deigenstrain_dargs[i].push_back(&getMaterialPropertyDerivative<RankTwoTensor>(
          eigenstrain_name, _coupled_moose_vars[i]->name()));

  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_out_of_plane_direction != 2 && _ndisp != 3)
    mooseError("For 2D simulations where the out-of-plane direction is x or y coordinate "
               "directions the number of supplied displacements must be three.");
  else if (_out_of_plane_direction == 2 && _ndisp != _mesh.dimension())
    mooseError("The number of displacement variables supplied must match the mesh dimension");

  if (_use_finite_deform_jacobian)
  {
    _deformation_gradient =
        &getMaterialProperty<RankTwoTensor>(_base_name + "deformation_gradient");
    _deformation_gradient_old =
        &getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient");
    _rotation_increment = &getMaterialProperty<RankTwoTensor>(_base_name + "rotation_increment");
  }

  // Error if volumetric locking correction is turned on for 1D problems
  if (_ndisp == 1 && _volumetric_locking_correction)
    mooseError("Volumetric locking correction should be set to false for 1-D problems.");

  // Generate warning when volumetric locking correction is used with second order elements
  if (_mesh.hasSecondOrderElements() && _volumetric_locking_correction)
    mooseWarning("Volumteric locking correction is not required for second order elements. Using "
                 "volumetric locking with second order elements could cause zigzag patterns in "
                 "stresses and strains.");
}

void
StressDivergenceTensors::initialSetup()
{
  // check if any of the eigenstrains provide derivatives wrt variables that are not coupled
  for (auto eigenstrain_name : getParam<std::vector<MaterialPropertyName>>("eigenstrain_names"))
    validateNonlinearCoupling<RankTwoTensor>(eigenstrain_name);

  // make sure the coordinate system is cartesioan
  if (getBlockCoordSystem() != Moose::COORD_XYZ)
    mooseError("The coordinate system in the Problem block must be set to XYZ for cartesian "
               "geometries.");
}

void
StressDivergenceTensors::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  if (_volumetric_locking_correction)
    computeAverageGradientTest();

  precalculateResidual();
  for (_i = 0; _i < _test.size(); ++_i)
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual();

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
      var->sys().solution().add_vector(_local_re, var->dofIndices());
  }
}

Real
StressDivergenceTensors::computeQpResidual()
{
  Real residual = _stress[_qp].row(_component) * _grad_test[_i][_qp];
  // volumetric locking correction
  if (_volumetric_locking_correction)
    residual += _stress[_qp].trace() / 3.0 *
                (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component));

  if (_ndisp != 3 && _out_of_plane_strain_coupled && _use_displaced_mesh)
  {
    const Real out_of_plane_thickness = std::exp((*_out_of_plane_strain)[_qp]);
    residual *= out_of_plane_thickness;
  }

  return residual;
}

void
StressDivergenceTensors::computeJacobian()
{
  if (_volumetric_locking_correction)
  {
    computeAverageGradientTest();
    computeAverageGradientPhi();
  }

  if (_use_finite_deform_jacobian)
  {
    _finite_deform_Jacobian_mult.resize(_qrule->n_points());

    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      computeFiniteDeformJacobian();

    ALEKernel::computeJacobian();
  }
  else
    Kernel::computeJacobian();
}

void
StressDivergenceTensors::computeOffDiagJacobian(const unsigned int jvar)
{
  if (_volumetric_locking_correction)
  {
    computeAverageGradientPhi();
    computeAverageGradientTest();
  }

  if (_use_finite_deform_jacobian)
  {
    _finite_deform_Jacobian_mult.resize(_qrule->n_points());

    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      computeFiniteDeformJacobian();

    ALEKernel::computeOffDiagJacobian(jvar);
  }
  else
    Kernel::computeOffDiagJacobian(jvar);
}

Real
StressDivergenceTensors::computeQpJacobian()
{
  if (_use_finite_deform_jacobian)
    return ElasticityTensorTools::elasticJacobian(_finite_deform_Jacobian_mult[_qp],
                                                  _component,
                                                  _component,
                                                  _grad_test[_i][_qp],
                                                  _grad_phi_undisplaced[_j][_qp]);

  Real sum_C3x3 = _Jacobian_mult[_qp].sum3x3();
  RealGradient sum_C3x1 = _Jacobian_mult[_qp].sum3x1();

  Real jacobian = 0.0;
  // B^T_i * C * B_j
  jacobian += ElasticityTensorTools::elasticJacobian(
      _Jacobian_mult[_qp], _component, _component, _grad_test[_i][_qp], _grad_phi[_j][_qp]);

  if (_volumetric_locking_correction)
  {
    // jacobian = Bbar^T_i * C * Bbar_j where Bbar = B + Bvol
    // jacobian = B^T_i * C * B_j + Bvol^T_i * C * Bvol_j +  Bvol^T_i * C * B_j + B^T_i * C *
    // Bvol_j

    // Bvol^T_i * C * Bvol_j
    jacobian += sum_C3x3 * (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component)) *
                (_avg_grad_phi[_j][_component] - _grad_phi[_j][_qp](_component)) / 9.0;

    // B^T_i * C * Bvol_j
    jacobian += sum_C3x1(_component) * _grad_test[_i][_qp](_component) *
                (_avg_grad_phi[_j][_component] - _grad_phi[_j][_qp](_component)) / 3.0;

    // Bvol^T_i * C * B_j
    RankTwoTensor phi;
    switch (_component)
    {
      case 0:
        phi(0, 0) = _grad_phi[_j][_qp](0);
        phi(0, 1) = phi(1, 0) = _grad_phi[_j][_qp](1);
        phi(0, 2) = phi(2, 0) = _grad_phi[_j][_qp](2);
        break;

      case 1:
        phi(1, 1) = _grad_phi[_j][_qp](1);
        phi(0, 1) = phi(1, 0) = _grad_phi[_j][_qp](0);
        phi(1, 2) = phi(2, 1) = _grad_phi[_j][_qp](2);
        break;

      case 2:
        phi(2, 2) = _grad_phi[_j][_qp](2);
        phi(0, 2) = phi(2, 0) = _grad_phi[_j][_qp](0);
        phi(1, 2) = phi(2, 1) = _grad_phi[_j][_qp](1);
        break;
    }

    jacobian += (_Jacobian_mult[_qp] * phi).trace() *
                (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component)) / 3.0;
  }

  if (_ndisp != 3 && _out_of_plane_strain_coupled && _use_displaced_mesh)
  {
    const Real out_of_plane_thickness = std::exp((*_out_of_plane_strain)[_qp]);
    jacobian *= out_of_plane_thickness;
  }

  return jacobian;
}

Real
StressDivergenceTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  // off-diagonal Jacobian with respect to a coupled displacement component
  for (unsigned int coupled_component = 0; coupled_component < _ndisp; ++coupled_component)
    if (jvar == _disp_var[coupled_component])
    {
      if (_out_of_plane_direction != 2)
      {
        if (coupled_component == _out_of_plane_direction)
          continue;
      }

      if (_use_finite_deform_jacobian)
        return ElasticityTensorTools::elasticJacobian(_finite_deform_Jacobian_mult[_qp],
                                                      _component,
                                                      coupled_component,
                                                      _grad_test[_i][_qp],
                                                      _grad_phi_undisplaced[_j][_qp]);

      const Real sum_C3x3 = _Jacobian_mult[_qp].sum3x3();
      const RealGradient sum_C3x1 = _Jacobian_mult[_qp].sum3x1();
      Real jacobian = 0.0;

      // B^T_i * C * B_j
      jacobian += ElasticityTensorTools::elasticJacobian(_Jacobian_mult[_qp],
                                                         _component,
                                                         coupled_component,
                                                         _grad_test[_i][_qp],
                                                         _grad_phi[_j][_qp]);

      if (_volumetric_locking_correction)
      {
        // jacobian = Bbar^T_i * C * Bbar_j where Bbar = B + Bvol
        // jacobian = B^T_i * C * B_j + Bvol^T_i * C * Bvol_j +  Bvol^T_i * C * B_j + B^T_i * C *
        // Bvol_j

        // Bvol^T_i * C * Bvol_j
        jacobian += sum_C3x3 * (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component)) *
                    (_avg_grad_phi[_j][coupled_component] - _grad_phi[_j][_qp](coupled_component)) /
                    9.0;

        // B^T_i * C * Bvol_j
        jacobian += sum_C3x1(_component) * _grad_test[_i][_qp](_component) *
                    (_avg_grad_phi[_j][coupled_component] - _grad_phi[_j][_qp](coupled_component)) /
                    3.0;

        // Bvol^T_i * C * B_i
        RankTwoTensor phi;
        for (unsigned int i = 0; i < 3; ++i)
          phi(coupled_component, i) = _grad_phi[_j][_qp](i);

        jacobian += (_Jacobian_mult[_qp] * phi).trace() *
                    (_avg_grad_test[_i][_component] - _grad_test[_i][_qp](_component)) / 3.0;
      }

      return jacobian;
    }

  // off-diagonal Jacobian with respect to a coupled out_of_plane_strain variable
  if (_out_of_plane_strain_coupled && jvar == _out_of_plane_strain_var)
    return _Jacobian_mult[_qp](
               _component, _component, _out_of_plane_direction, _out_of_plane_direction) *
           _grad_test[_i][_qp](_component) * _phi[_j][_qp];

  // bail out if jvar is not coupled
  if (getJvarMap()[jvar] < 0)
    return 0.0;

  // off-diagonal Jacobian with respect to any other coupled variable
  const unsigned int cvar = mapJvarToCvar(jvar);
  RankTwoTensor total_deigenstrain;
  for (const auto deigenstrain_darg : _deigenstrain_dargs[cvar])
    total_deigenstrain += (*deigenstrain_darg)[_qp];

  return -((_Jacobian_mult[_qp] * total_deigenstrain) *
           _grad_test[_i][_qp])(_component)*_phi[_j][_qp];
}

void
StressDivergenceTensors::computeFiniteDeformJacobian()
{
  usingTensorIndices(i_, j_, k_, l_);
  const auto I = RankTwoTensor::Identity();
  const RankFourTensor I2 = I.times<i_, k_, j_, l_>(I);

  // Bring back to unrotated config
  const RankTwoTensor unrotated_stress =
      (*_rotation_increment)[_qp].transpose() * _stress[_qp] * (*_rotation_increment)[_qp];

  // Incremental deformation gradient Fhat
  const RankTwoTensor Fhat =
      (*_deformation_gradient)[_qp] * (*_deformation_gradient_old)[_qp].inverse();
  const RankTwoTensor Fhatinv = Fhat.inverse();

  const RankTwoTensor rot_times_stress = (*_rotation_increment)[_qp] * unrotated_stress;
  const RankFourTensor dstress_drot =
      I.times<i_, k_, j_, l_>(rot_times_stress) + I.times<j_, k_, i_, l_>(rot_times_stress);
  const RankFourTensor rot_rank_four =
      (*_rotation_increment)[_qp].times<i_, k_, j_, l_>((*_rotation_increment)[_qp]);
  const RankFourTensor drot_dUhatinv = Fhat.times<i_, k_, j_, l_>(I);

  const RankTwoTensor A = I - Fhatinv;

  // Ctilde = Chat^-1 - I
  const RankTwoTensor Ctilde = A * A.transpose() - A - A.transpose();
  const RankFourTensor dCtilde_dFhatinv =
      -I.times<i_, k_, j_, l_>(A) - I.times<j_, k_, i_, l_>(A) + I2 + I.times<j_, k_, i_, l_>(I);

  // Second order approximation of Uhat - consistent with strain increment definition
  // const RankTwoTensor Uhat = I - 0.5 * Ctilde - 3.0/8.0 * Ctilde * Ctilde;

  RankFourTensor dUhatinv_dCtilde =
      0.5 * I2 - 1.0 / 8.0 * (I.times<i_, k_, j_, l_>(Ctilde) + Ctilde.times<i_, k_, j_, l_>(I));
  RankFourTensor drot_dFhatinv = drot_dUhatinv * dUhatinv_dCtilde * dCtilde_dFhatinv;

  drot_dFhatinv -= Fhat.times<i_, k_, j_, l_>((*_rotation_increment)[_qp].transpose());
  _finite_deform_Jacobian_mult[_qp] = dstress_drot * drot_dFhatinv;

  const RankFourTensor dstrain_increment_dCtilde =
      -0.5 * I2 + 0.25 * (I.times<i_, k_, j_, l_>(Ctilde) + Ctilde.times<i_, k_, j_, l_>(I));
  _finite_deform_Jacobian_mult[_qp] +=
      rot_rank_four * _Jacobian_mult[_qp] * dstrain_increment_dCtilde * dCtilde_dFhatinv;
  _finite_deform_Jacobian_mult[_qp] += Fhat.times<j_, k_, i_, l_>(_stress[_qp]);

  const RankFourTensor dFhat_dFhatinv = -Fhat.times<i_, k_, j_, l_>(Fhat.transpose());
  const RankTwoTensor dJ_dFhatinv = dFhat_dFhatinv.innerProductTranspose(Fhat.ddet());

  // Component from Jacobian derivative
  _finite_deform_Jacobian_mult[_qp] += _stress[_qp].times<i_, j_, k_, l_>(dJ_dFhatinv);

  // Derivative of Fhatinv w.r.t. undisplaced coordinates
  const RankTwoTensor Finv = (*_deformation_gradient)[_qp].inverse();
  const RankFourTensor dFhatinv_dGradu = -Fhatinv.times<i_, k_, j_, l_>(Finv.transpose());
  _finite_deform_Jacobian_mult[_qp] = _finite_deform_Jacobian_mult[_qp] * dFhatinv_dGradu;
}

void
StressDivergenceTensors::computeAverageGradientTest()
{
  // Calculate volume averaged value of shape function derivative
  _avg_grad_test.resize(_test.size());
  for (_i = 0; _i < _test.size(); ++_i)
  {
    _avg_grad_test[_i].resize(3);
    _avg_grad_test[_i][_component] = 0.0;
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
      _avg_grad_test[_i][_component] += _grad_test[_i][_qp](_component) * _JxW[_qp] * _coord[_qp];

    _avg_grad_test[_i][_component] /= _current_elem_volume;
  }
}

void
StressDivergenceTensors::computeAverageGradientPhi()
{
  // Calculate volume average derivatives for phi
  _avg_grad_phi.resize(_phi.size());
  for (_i = 0; _i < _phi.size(); ++_i)
  {
    _avg_grad_phi[_i].resize(3);
    for (unsigned int component = 0; component < _mesh.dimension(); ++component)
    {
      _avg_grad_phi[_i][component] = 0.0;
      for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
        _avg_grad_phi[_i][component] += _grad_phi[_i][_qp](component) * _JxW[_qp] * _coord[_qp];

      _avg_grad_phi[_i][component] /= _current_elem_volume;
    }
  }
}
