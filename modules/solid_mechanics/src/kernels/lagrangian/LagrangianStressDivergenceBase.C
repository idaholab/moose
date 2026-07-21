//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LagrangianStressDivergenceBase.h"
#include "RankFourTensor.h"

InputParameters
LagrangianStressDivergenceBase::validParams()
{
  InputParameters params = KernelScalarBase::validParams();

  params.addRequiredParam<unsigned int>("component", "Which direction this kernel acts in");
  params.addRequiredCoupledVar("displacements", "The displacement components");

  params.addParam<bool>("large_kinematics", false, "Use large displacement kinematics");
  params.addParam<bool>("stabilize_strain", false, "Average the volumetric strains");
  MooseEnum F_bar_mode("total incremental", "total");
  params.addParam<MooseEnum>(
      "F_bar_mode",
      F_bar_mode,
      "Which F gets the F-bar volumetric correction (must match the strain calc's setting). "
      "'total' (default) reproduces existing behavior; 'incremental' makes the F-bar element-"
      "average operate on the incremental F so cumulative strain matches OLD's "
      "`ComputeFiniteStrain` + `volumetric_locking_correction = true`.");

  params.addParam<std::string>("base_name", "Material property base name");

  params.addCoupledVar("temperature",
                       "The name of the temperature variable used in the "
                       "ComputeThermalExpansionEigenstrain.  (Not required for "
                       "simulations without temperature coupling.)");

  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names",
      {},
      "List of eigenstrains used in the strain calculation. Used for computing their derivatives "
      "for off-diagonal Jacobian terms.");

  params.addCoupledVar("out_of_plane_strain",
                       "The out-of-plane strain variable for weak plane stress formulation.");

  return params;
}

LagrangianStressDivergenceBase::LagrangianStressDivergenceBase(const InputParameters & parameters)
  : JvarMapKernelInterface<DerivativeMaterialInterface<KernelScalarBase>>(parameters),
    _large_kinematics(getParam<bool>("large_kinematics")),
    _stabilize_strain(getParam<bool>("stabilize_strain")),
    _F_bar_mode(getParam<MooseEnum>("F_bar_mode") == "incremental" ? FBarMode::Incremental
                                                                   : FBarMode::Total),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _alpha(getParam<unsigned int>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp_nums(_ndisp),
    _avg_grad_trial(_ndisp),
    _avg_grad_spatial_phi(_ndisp),
    _avg_test_phi_cross(3, std::vector<std::vector<std::vector<Real>>>(3)),
    _F_ust(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "unstabilized_deformation_gradient")),
    _F_ust_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name +
                                                           "unstabilized_deformation_gradient")),
    _F_avg(getMaterialPropertyByName<RankTwoTensor>(_base_name + "average_deformation_gradient")),
    _f_inv(getMaterialPropertyByName<RankTwoTensor>(_base_name +
                                                    "inverse_incremental_deformation_gradient")),
    _F_inv(getMaterialPropertyByName<RankTwoTensor>(_base_name + "inverse_deformation_gradient")),
    _F(getMaterialPropertyByName<RankTwoTensor>(_base_name + "deformation_gradient")),
    _F_actual(getMaterialPropertyByName<RankTwoTensor>(_base_name + "actual_deformation_gradient")),
    _d_deformation_gradient_increment_d_F(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_spatial_deformation_gradient_increment_d_deformation_gradient")),
    _d_F_d_grad_u(getMaterialPropertyByName<RankFourTensor>(
        _base_name + "d_deformation_gradient_d_grad_displacement")),
    _d_F_stab_d_F_ust(
        getMaterialPropertyByName<RankFourTensor>(_base_name + "d_F_stab_d_F_unstabilized")),
    _d_F_stab_d_F_avg(
        getMaterialPropertyByName<RankFourTensor>(_base_name + "d_F_stab_d_F_average")),
    _temperature(isCoupled("temperature") ? getVar("temperature", 0) : nullptr),
    _out_of_plane_strain(isCoupled("out_of_plane_strain") ? getVar("out_of_plane_strain", 0)
                                                          : nullptr)
{
  // Do the vector coupling of the displacements
  for (unsigned int i = 0; i < _ndisp; i++)
    _disp_nums[i] = coupled("displacements", i);

  // We need to use identical discretizations for all displacement components
  auto order_x = getVar("displacements", 0)->order();
  for (unsigned int i = 1; i < _ndisp; i++)
  {
    if (getVar("displacements", i)->order() != order_x)
      mooseError("The Lagrangian StressDivergence kernels require equal "
                 "order interpolation for all displacements.");
  }

  // fetch eigenstrain derivatives
  const auto nvar = _coupled_moose_vars.size();
  _deigenstrain_dargs.resize(nvar);
  for (std::size_t i = 0; i < nvar; ++i)
    for (auto eigenstrain_name : getParam<std::vector<MaterialPropertyName>>("eigenstrain_names"))
      _deigenstrain_dargs[i].push_back(&getMaterialPropertyDerivative<RankTwoTensor>(
          eigenstrain_name, _coupled_moose_vars[i]->name()));

  // The direct-chain temperature off-diagonal Jacobian needs d_sigma/d_eigenstrain. Only
  // fetch it when eigenstrains are coupled -- otherwise the temperature Jacobian short-
  // circuits to zero and this property would be a needless dependency that other Cauchy-
  // providing materials would have to publish.
  if (!getParam<std::vector<MaterialPropertyName>>("eigenstrain_names").empty())
    _dcauchy_stress_d_eigenstrain =
        &getMaterialPropertyByName<RankFourTensor>(_base_name + "dcauchy_stress_d_eigenstrain");

  // The F-bar spatial push-forward consumes F_ust^{-1}/det(F_ust) published by the strain calc.
  // Fetch them (and thereby mark them active for the strain material's isPropertyActive gate)
  // only when stabilization is on -- the only mode where the push-forward runs.
  if (_stabilize_strain)
  {
    _F_ust_inv = &getMaterialPropertyByName<RankTwoTensor>(
        _base_name + "inverse_unstabilized_deformation_gradient");
    _F_ust_det =
        &getMaterialPropertyByName<Real>(_base_name + "det_unstabilized_deformation_gradient");
    _d_nl_fbar = &getMaterialPropertyByName<RankFourTensor>(_base_name + "d_nl_fbar_operator");
  }
}

void
LagrangianStressDivergenceBase::precalculateJacobian()
{
  // Skip if we are not doing stabilization
  if (!_stabilize_strain)
    return;

  // We need the gradients of shape functions in the reference frame
  _fe_problem.prepareShapes(_var.number(), _tid);
  _avg_grad_trial[_alpha].resize(_phi.size());
  precalculateJacobianDisplacement(_alpha);
}

void
LagrangianStressDivergenceBase::precalculateOffDiagJacobian(unsigned int jvar)
{
  // Skip if we are not doing stabilization
  if (!_stabilize_strain)
    return;

  for (auto beta : make_range(_ndisp))
    if (jvar == _disp_nums[beta])
    {
      // We need the gradients of shape functions in the reference frame
      _fe_problem.prepareShapes(jvar, _tid);
      _avg_grad_trial[beta].resize(_phi.size());
      precalculateJacobianDisplacement(beta);
    }
}

Real
LagrangianStressDivergenceBase::computeQpJacobian()
{
  return computeQpJacobianDisplacement(_alpha, _alpha);
}

RankTwoTensor
LagrangianStressDivergenceBase::deltaPK1NonLocalFBar(const RankTwoTensor & delta_F_avg) const
{
  if (!_stabilize_strain)
    return RankTwoTensor();
  // `_d_nl_fbar` (the composed non-local operator) and F_ust^{-1}/det come from the stress and
  // strain materials as per-qp properties -- no kernel-side cache to refresh.
  const RankTwoTensor delta_sigma_nl = (*_d_nl_fbar)[_qp] * delta_F_avg;
  if (_large_kinematics)
    return (*_F_ust_det)[_qp] * delta_sigma_nl * (*_F_ust_inv)[_qp].transpose();
  return delta_sigma_nl;
}

Real
LagrangianStressDivergenceBase::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Bail if jvar not coupled
  if (getJvarMap()[jvar] < 0)
    return 0.0;

  // Off diagonal terms for other displacements
  for (auto beta : make_range(_ndisp))
    if (jvar == _disp_nums[beta])
      return computeQpJacobianDisplacement(_alpha, beta);

  // Off diagonal temperature term due to eigenstrain
  if (_temperature && jvar == _temperature->number())
    return computeQpJacobianTemperature(mapJvarToCvar(jvar));

  // Off diagonal term due to weak plane stress
  if (_out_of_plane_strain && jvar == _out_of_plane_strain->number())
    return computeQpJacobianOutOfPlaneStrain();

  return 0;
}
