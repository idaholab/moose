//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityUpdate.h"

#include "libmesh/utility.h"
#include "Conversion.h"
#include "MooseException.h"

InputParameters
CrystalPlasticityUpdate::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Crystal Plasticity base class: handles the Newton iteration over the stress residual and "
      "calculates the Jacobian based on constitutive laws provided by inheriting classes");
  params.addParam<std::string>(
      "base_name",
      "Optional parameter that allows the user to define multiple mechanics material systems on "
      "the same block, i.e. for multiple phases");

  // The return stress increment classes are intended to be iterative materials, so must set compute
  // = false for all inheriting classes
  params.set<bool>("compute") = false;
  params.suppressParameter<bool>("compute");

  params.addRequiredParam<unsigned int>(
      "number_slip_systems",
      "The total number of possible active slip systems for the crystalline material");
  params.addRequiredParam<FileName>("slip_sys_file_name",
                                    "Name of the file containing the slip systems");
  params.addParam<Real>("number_cross_slip_directions",
                        0,
                        "Quanity of unique slip directions, used to determine cross slip familes");
  params.addParam<Real>("number_cross_slip_planes",
                        0,
                        "Quanity of slip planes belonging to a single cross slip direction; used "
                        "to determine cross slip families");

  params.addParam<Real>("rtol", 1e-6, "Constitutive stress residual relative tolerance");
  params.addParam<Real>("abs_tol", 1e-6, "Constitutive stress residual absolute tolerance");
  params.addParam<Real>(
      "stol", 1e-2, "Constitutive internal state variable relative change tolerance");
  params.addParam<Real>("slip_increment_tolerance", 2e-2, "Maximum allowable slip in an increment");
  params.addParam<Real>(
      "resistance_tol", 1.0e-2, "Constitutive slip system resistance residual tolerance");
  params.addParam<Real>(
      "zero_tol", 1e-12, "Tolerance for residual check when variable value is zero");
  params.addParam<unsigned int>("maxiter", 100, "Maximum number of iterations for stress update");
  params.addParam<unsigned int>(
      "maxiter_state_variable", 100, "Maximum number of iterations for state variable update");
  MooseEnum tan_mod_options("exact none", "none");
  params.addParam<MooseEnum>("tan_mod_type",
                             tan_mod_options,
                             "Type of tangent moduli for preconditioner: default elastic");
  params.addParam<unsigned int>(
      "maximum_substep_iteration", 1, "Maximum number of substep iteration");
  params.addParam<bool>("use_line_search", false, "Use line search in constitutive update");
  params.addParam<Real>("min_line_search_step_size", 0.01, "Minimum line search step size");
  params.addParam<Real>("line_search_tol", 0.5, "Line search bisection method tolerance");
  params.addParam<unsigned int>(
      "line_search_maxiter", 20, "Line search bisection method maximum number of iteration");
  MooseEnum line_search_method("CUT_HALF BISECTION", "CUT_HALF");
  params.addParam<MooseEnum>(
      "line_search_method", line_search_method, "The method used in line search");

  return params;
}

CrystalPlasticityUpdate::CrystalPlasticityUpdate(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor")),

    _number_slip_systems(getParam<unsigned int>("number_slip_systems")),
    _slip_sys_file_name(getParam<FileName>("slip_sys_file_name")),
    _number_cross_slip_directions(getParam<Real>("number_cross_slip_directions")),
    _number_cross_slip_planes(getParam<Real>("number_cross_slip_planes")),

    _rtol(getParam<Real>("rtol")),
    _abs_tol(getParam<Real>("abs_tol")),
    _rel_state_var_tol(getParam<Real>("stol")),
    _slip_incr_tol(getParam<Real>("slip_increment_tolerance")),
    _resistance_tol(getParam<Real>("resistance_tol")),
    _zero_tol(getParam<Real>("zero_tol")),
    _maxiter(getParam<unsigned int>("maxiter")),
    _maxiterg(getParam<unsigned int>("maxiter_state_variable")),
    _tan_mod_type(getParam<MooseEnum>("tan_mod_type")),
    _max_substep_iter(getParam<unsigned int>("maximum_substep_iteration")),
    _use_line_search(getParam<bool>("use_line_search")),
    _min_line_search_step_size(getParam<Real>("min_line_search_step_size")),
    _line_search_tolerance(getParam<Real>("line_search_tol")),
    _line_search_max_iterations(getParam<unsigned int>("line_search_maxiter")),
    _line_search_method(getParam<MooseEnum>("line_search_method")),
    _plastic_deformation_gradient(declareProperty<RankTwoTensor>("fp")),
    _plastic_deformation_gradient_old(getMaterialPropertyOld<RankTwoTensor>("fp")),
    _deformation_gradient(getMaterialProperty<RankTwoTensor>("deformation_gradient")),
    _deformation_gradient_old(getMaterialPropertyOld<RankTwoTensor>("deformation_gradient")),
    _pk2(declareProperty<RankTwoTensor>("pk2")), // 2nd Piola Kirchoff Stress
    _pk2_old(getMaterialPropertyOld<RankTwoTensor>("pk2")),
    _total_lagrangian_strain(
        declareProperty<RankTwoTensor>("total_lagrangian_strain")), // Lagrangian strain

    _slip_direction(_number_slip_systems * LIBMESH_DIM),
    _slip_plane_normal(_number_slip_systems * LIBMESH_DIM),
    _flow_direction(declareProperty<std::vector<RankTwoTensor>>("flow_direction")),
    _tau(declareProperty<std::vector<Real>>("applied_shear_stress")),
    _update_rotation(declareProperty<RankTwoTensor>("update_rot")),
    _crysrot(getMaterialProperty<RankTwoTensor>(
        "crysrot")) // defined in the elasticity tensor classes for crystal plasticity
{
  _error_tolerance = false;
  _substep_dt = 0.0;
  _delta_deformation_gradient.zero();

  getSlipSystems();
  sortCrossSlipFamilies();

  if (parameters.isParamSetByUser("number_cross_slip_directions"))
    _calculate_cross_slip = true;
  else
    _calculate_cross_slip = false;
}

void
CrystalPlasticityUpdate::initQpStatefulProperties()
{
  _plastic_deformation_gradient[_qp].zero();
  _plastic_deformation_gradient[_qp].addIa(1.0);

  _pk2[_qp].zero();
  _tau[_qp].resize(_number_slip_systems);

  _total_lagrangian_strain[_qp].zero();

  _update_rotation[_qp].zero();
  _update_rotation[_qp].addIa(1.0);

  _flow_direction[_qp].resize(_number_slip_systems);
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    _flow_direction[_qp][i].zero();
    _tau[_qp][i] = 0.0;
  }
}

void
CrystalPlasticityUpdate::updateStress(RankTwoTensor & cauchy_stress, RankFourTensor & jacobian_mult)
{
  // Does not support face/boundary material property calculation
  if (isBoundaryMaterial())
    return;

  // Initialize substepping variables
  unsigned int substep_iter = 1;
  unsigned int num_substep = 1;

  _temporary_deformation_gradient_old = _deformation_gradient_old[_qp];
  if (_temporary_deformation_gradient_old.det() == 0)
    _temporary_deformation_gradient_old.addIa(1.0);

  _delta_deformation_gradient = _deformation_gradient[_qp] - _temporary_deformation_gradient_old;

  // Calculate the schmid tensor for the current state of the crystal lattice
  calculateFlowDirection();

  do
  {
    _error_tolerance = false;
    preSolveQp();

    _substep_dt = _dt / num_substep;

    for (unsigned int istep = 0; istep < num_substep; ++istep)
    {
      _temporary_deformation_gradient =
          (static_cast<Real>(istep) + 1) / num_substep * _delta_deformation_gradient;
      _temporary_deformation_gradient += _temporary_deformation_gradient_old;

      solveQp();

      if (_error_tolerance)
      {
        substep_iter++;
        num_substep *= 2;
        break;
      }
    }

    if (substep_iter > _max_substep_iter && _error_tolerance)
      mooseException("CrystalPlasticityUpdate: Constitutive failure");
  } while (_error_tolerance);

  postSolveQp(cauchy_stress, jacobian_mult);
}

void
CrystalPlasticityUpdate::preSolveQp()
{
  setInitialConstitutiveVariableValues();

  _pk2[_qp] = _pk2_old[_qp];
  _inverse_plastic_deformation_grad_old = _plastic_deformation_gradient_old[_qp].inverse();
}

void
CrystalPlasticityUpdate::solveQp()
{
  setSubstepConstitutiveVariableValues();
  _inverse_plastic_deformation_grad = _inverse_plastic_deformation_grad_old;

  solveStateVariables();
  if (_error_tolerance)
    return; // pop back up and take a smaller substep

  updateSubstepConstitutiveVariableValues();

  // save off the old F^{p} inverse now that have converged on the stress and state variables
  _inverse_plastic_deformation_grad_old = _inverse_plastic_deformation_grad;
}

void
CrystalPlasticityUpdate::postSolveQp(RankTwoTensor & cauchy_stress, RankFourTensor & jacobian_mult)
{
  cauchy_stress = _elastic_deformation_gradient * _pk2[_qp] *
                  _elastic_deformation_gradient.transpose() / _elastic_deformation_gradient.det();

  calcTangentModuli(jacobian_mult);

  _total_lagrangian_strain[_qp] =
      _deformation_gradient[_qp].transpose() * _deformation_gradient[_qp] -
      RankTwoTensor::Identity();
  _total_lagrangian_strain[_qp] = _total_lagrangian_strain[_qp] * 0.5;

  // Calculate crystal rotation to track separately
  RankTwoTensor rot;
  _deformation_gradient[_qp].getRUDecompositionRotation(rot);
  _update_rotation[_qp] = rot * _crysrot[_qp];
}

void
CrystalPlasticityUpdate::solveStateVariables()
{
  unsigned int iteration;
  bool iter_flag = true;

  iteration = 0;
  // Check for slip system resistance update tolerance
  while (iter_flag && iteration < _maxiterg)
  {
    solveStress();
    if (_error_tolerance)
      return;

    _plastic_deformation_gradient[_qp] =
        _inverse_plastic_deformation_grad.inverse(); // the postSoveStress

    // Update slip system resistance and state variable after the stress has been finalized
    updateConstitutiveSlipSystemResistanceAndVariables(_error_tolerance);
    if (_error_tolerance)
      return;

    iter_flag = areConstitutiveStateVariablesConverged(); // returns false if values are converged
                                                          // and good to go

    if (iter_flag)
    {
#ifdef DEBUG
      mooseWarning("CrystalPlasticityUpdate: State variables (or the system resistance) did not "
                   "converge at element ",
                   _current_elem->id(),
                   " and qp ",
                   _qp,
                   "\n");
#endif
    }
    iteration++;
  }

  if (iteration == _maxiterg)
  {
#ifdef DEBUG
    mooseWarning("CrystalPlasticityUpdate: Hardness Integration error. Reached the maximum number "
                 "of iterations to solve for the state variables at element ",
                 _current_elem->id(),
                 " and qp ",
                 _qp,
                 "\n");
#endif
    _error_tolerance = true;
  }
}

void
CrystalPlasticityUpdate::solveStress()
{
  unsigned int iteration = 0;
  RankTwoTensor dpk2;
  Real rnorm, rnorm0, rnorm_prev;

  // Calculate stress residual
  calculateResidualAndJacobian();
  if (_error_tolerance)
  {
#ifdef DEBUG
    mooseWarning("CrystalPlasticityUpdate: Slip increment exceeds tolerance - Element number ",
                 _current_elem->id(),
                 " Gauss point = ",
                 _qp);
#endif
    return;
  }

  rnorm = _residual_tensor.L2norm();
  rnorm0 = rnorm;

  // Check for stress residual tolerance; different from user object version which
  // compares the absolute tolerance of only the original rnorm value
  while (rnorm > _rtol * rnorm0 && rnorm > _abs_tol && iteration < _maxiter)
  {
    // Calculate stress increment
    dpk2 = -_jacobian.invSymm() * _residual_tensor;
    _pk2[_qp] = _pk2[_qp] + dpk2;

    calculateResidualAndJacobian();

    if (_error_tolerance)
    {
#ifdef DEBUG
      mooseWarning("CrystalPlasticityUpdate: Slip increment exceeds tolerance - Element number ",
                   _current_elem->id(),
                   " Gauss point = ",
                   _qp);
#endif
      return;
    }

    rnorm_prev = rnorm;
    rnorm = _residual_tensor.L2norm();

    if (_use_line_search && rnorm > rnorm_prev && !lineSearchUpdate(rnorm_prev, dpk2))
    {
#ifdef DEBUG
      mooseWarning("CrystalPlasticityUpdate: Failed with line search");
#endif
      _error_tolerance = true;
      return;
    }

    if (_use_line_search)
      rnorm = _residual_tensor.L2norm();

    iteration++;
  }

  if (iteration >= _maxiter)
  {
#ifdef DEBUG
    mooseWarning("CrystalPlasticityUpdate: Stress Integration error rmax = ",
                 rnorm,
                 " and the tolerance is ",
                 _rtol * rnorm0,
                 "when the rnorm0 value is ",
                 rnorm0,
                 "for element ",
                 _current_elem->id(),
                 " and qp ",
                 _qp);
#endif
    _error_tolerance = true;
  }
}

// Calculates stress residual equation and jacobian
void
CrystalPlasticityUpdate::calculateResidualAndJacobian()
{
  calcResidual();
  if (_error_tolerance)
    return;
  calcJacobian();
}

void
CrystalPlasticityUpdate::calcResidual()
{
  RankTwoTensor ce, elastic_strain, ce_pk2, equivalent_slip_increment, pk2_new;

  equivalent_slip_increment.zero();

  for (unsigned int i = 0; i < _number_slip_systems; ++i)
    _tau[_qp][i] = _pk2[_qp].doubleContraction(_flow_direction[_qp][i]);

  // Call the overwritten method in the inheriting class that contains the constitutive model
  calculateConstitutiveEquivalentSlipIncrement(equivalent_slip_increment, _error_tolerance);

  if (_error_tolerance)
    return;

  RankTwoTensor residual_equivalent_slip_increment =
      RankTwoTensor::Identity() - equivalent_slip_increment;
  _inverse_plastic_deformation_grad =
      _inverse_plastic_deformation_grad_old * residual_equivalent_slip_increment;

  _elastic_deformation_gradient =
      _temporary_deformation_gradient * _inverse_plastic_deformation_grad;

  ce = _elastic_deformation_gradient.transpose() * _elastic_deformation_gradient;
  elastic_strain = ce - RankTwoTensor::Identity();
  elastic_strain *= 0.5;

  pk2_new = _elasticity_tensor[_qp] * elastic_strain;
  _residual_tensor = _pk2[_qp] - pk2_new;
}

void
CrystalPlasticityUpdate::calcJacobian()
{
  RankFourTensor dfedfpinv, deedfe, dfpinvdpk2;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        dfedfpinv(i, j, k, j) = _temporary_deformation_gradient(i, k);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        deedfe(i, j, k, i) = deedfe(i, j, k, i) + _elastic_deformation_gradient(k, j) * 0.5;
        deedfe(i, j, k, j) = deedfe(i, j, k, j) + _elastic_deformation_gradient(k, i) * 0.5;
      }

  calculateTotalPlasticDeformationGradientDerivative(dfpinvdpk2);

  _jacobian =
      RankFourTensor::IdentityFour() - (_elasticity_tensor[_qp] * deedfe * dfedfpinv * dfpinvdpk2);
}

void
CrystalPlasticityUpdate::calculateTotalPlasticDeformationGradientDerivative(
    RankFourTensor & dfpinvdpk2)
{
  calculateConstitutivePlasticDeformationGradientDerivative(
      dfpinvdpk2, _flow_direction[_qp], _number_slip_systems);
}

void
CrystalPlasticityUpdate::calculateConstitutivePlasticDeformationGradientDerivative(
    RankFourTensor & dfpinvdpk2,
    std::vector<RankTwoTensor> & schmid_tensor,
    const unsigned int & number_dislocation_systems,
    unsigned int /*slip_model_number*/)
{
  std::vector<Real> dslip_dtau(number_dislocation_systems, 0.0);
  std::vector<RankTwoTensor> dtaudpk2(number_dislocation_systems);
  std::vector<RankTwoTensor> dfpinvdslip(number_dislocation_systems);

  calculateConstitutiveSlipDerivative(dslip_dtau);

  for (unsigned int j = 0; j < number_dislocation_systems; ++j)
  {
    dtaudpk2[j] = schmid_tensor[j];
    dfpinvdslip[j] = -_inverse_plastic_deformation_grad_old * schmid_tensor[j];
    dfpinvdpk2 += (dfpinvdslip[j] * dslip_dtau[j] * _substep_dt).outerProduct(dtaudpk2[j]);
  }
}

void
CrystalPlasticityUpdate::calcTangentModuli(RankFourTensor & jacobian_mult)
{
  switch (_tan_mod_type)
  {
    case 0:
      elastoPlasticTangentModuli(jacobian_mult);
      break;
    default:
      elasticTangentModuli(jacobian_mult);
  }
}

void
CrystalPlasticityUpdate::elastoPlasticTangentModuli(RankFourTensor & jacobian_mult)
{
  RankFourTensor tan_mod;
  RankTwoTensor pk2fet, fepk2;
  RankFourTensor deedfe, dsigdpk2dfe, dfedf;

  // Fill in the matrix stiffness material property
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        deedfe(i, j, k, i) = deedfe(i, j, k, i) + _elastic_deformation_gradient(k, j) * 0.5;
        deedfe(i, j, k, j) = deedfe(i, j, k, j) + _elastic_deformation_gradient(k, i) * 0.5;
      }

  dsigdpk2dfe = _elastic_deformation_gradient.mixedProductIkJl(_elastic_deformation_gradient) *
                _elasticity_tensor[_qp] * deedfe;

  pk2fet = _pk2[_qp] * _elastic_deformation_gradient.transpose();
  fepk2 = _elastic_deformation_gradient * _pk2[_qp];

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
      {
        tan_mod(i, j, i, l) += pk2fet(l, j);
        tan_mod(i, j, j, l) += fepk2(i, l);
      }

  tan_mod += dsigdpk2dfe;

  Real je = _elastic_deformation_gradient.det();
  if (je > 0.0)
    tan_mod /= je;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
        dfedf(i, j, i, l) = _inverse_plastic_deformation_grad(l, j);

  jacobian_mult = tan_mod * dfedf;
}

void
CrystalPlasticityUpdate::elasticTangentModuli(RankFourTensor & jacobian_mult)
{
  // update jacobian_mult
  jacobian_mult = _elasticity_tensor[_qp];
}

bool
CrystalPlasticityUpdate::lineSearchUpdate(const Real rnorm_prev, const RankTwoTensor dpk2)
{
  switch (_line_search_method)
  {
    case 0: // CUT_HALF
    {
      Real rnorm;
      Real step = 1.0;

      do
      {
        _pk2[_qp] = _pk2[_qp] - step * dpk2;
        step /= 2.0;
        _pk2[_qp] = _pk2[_qp] + step * dpk2;

        calcResidual();
        rnorm = _residual_tensor.L2norm();
      } while (rnorm > rnorm_prev && step > _min_line_search_step_size);

      // has norm improved or is the step still above minumum search step size?
      return (rnorm <= rnorm_prev || step > _min_line_search_step_size);
    }

    case 1: // BISECTION
    {
      unsigned int count = 0;
      Real step_a = 0.0;
      Real step_b = 1.0;
      Real step = 1.0;
      Real s_m = 1000.0;
      Real rnorm = 1000.0;

      calcResidual();
      Real s_b = _residual_tensor.doubleContraction(dpk2);
      Real rnorm1 = _residual_tensor.L2norm();
      _pk2[_qp] = _pk2[_qp] - dpk2;
      calcResidual();
      Real s_a = _residual_tensor.doubleContraction(dpk2);
      Real rnorm0 = _residual_tensor.L2norm();
      _pk2[_qp] = _pk2[_qp] + dpk2;

      if ((rnorm1 / rnorm0) < _line_search_tolerance || s_a * s_b > 0)
      {
        calcResidual();
        return true;
      }

      while ((rnorm / rnorm0) > _line_search_tolerance && count < _line_search_max_iterations)
      {
        _pk2[_qp] = _pk2[_qp] - step * dpk2;
        step = 0.5 * (step_b + step_a);
        _pk2[_qp] = _pk2[_qp] + step * dpk2;
        calcResidual();
        s_m = _residual_tensor.doubleContraction(dpk2);
        rnorm = _residual_tensor.L2norm();

        if (s_m * s_a < 0.0)
        {
          step_b = step;
          s_b = s_m;
        }
        if (s_m * s_b < 0.0)
        {
          step_a = step;
          s_a = s_m;
        }
        count++;
      }

      // below tolerance and max iterations?
      return ((rnorm / rnorm0) < _line_search_tolerance && count < _line_search_max_iterations);
    }

    default:
      mooseError("Line search method is not provided.");
  }
}

void
CrystalPlasticityUpdate::calculateFlowDirection()
{
  calculateSchmidTensor(
      _number_slip_systems, _slip_plane_normal, _slip_direction, _flow_direction[_qp]);
}

void
CrystalPlasticityUpdate::calculateSchmidTensor(const unsigned int & number_dislocation_systems,
                                               const DenseVector<Real> & plane_normal_vector,
                                               const DenseVector<Real> & direction_vector,
                                               std::vector<RankTwoTensor> & schmid_tensor)
{
  DenseVector<Real> local_direction_vector(LIBMESH_DIM * number_dislocation_systems),
      local_plane_normal(LIBMESH_DIM * number_dislocation_systems);

  // Update slip direction and normal with crystal orientation
  for (unsigned int i = 0; i < number_dislocation_systems; ++i)
  {
    unsigned int system = i * LIBMESH_DIM;
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      local_direction_vector(system + j) = 0.0;
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        local_direction_vector(system + j) =
            local_direction_vector(system + j) + _crysrot[_qp](j, k) * direction_vector(system + k);
      }
    }

    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      local_plane_normal(system + j) = 0.0;
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        local_plane_normal(system + j) =
            local_plane_normal(system + j) + _crysrot[_qp](j, k) * plane_normal_vector(system + k);
    }
  }

  // Calculate Schmid tensor and resolved shear stresses
  for (unsigned int i = 0; i < number_dislocation_systems; ++i)
  {
    unsigned int system = i * LIBMESH_DIM;
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        schmid_tensor[i](j, k) =
            local_direction_vector(system + j) * local_plane_normal(system + k);
      }
  }
}

void
CrystalPlasticityUpdate::getSlipSystems()
{
  bool orthonormal_error = false;

  getPlaneNormalAndDirectionVectors(_slip_sys_file_name,
                                    _number_slip_systems,
                                    _slip_plane_normal,
                                    _slip_direction,
                                    orthonormal_error);

  if (orthonormal_error)
    mooseError("CrystalPlasticityUpdate Error: The slip system file contains a slip direction and "
               "plane normal pair that are not orthonormal");
}

void
CrystalPlasticityUpdate::getPlaneNormalAndDirectionVectors(
    const FileName & vector_file_name,
    const unsigned int & number_dislocation_systems,
    DenseVector<Real> & plane_normal_vector,
    DenseVector<Real> & direction_vector,
    bool & orthonormal_error)
{
  Real vec[LIBMESH_DIM];
  std::ifstream fileslipsys;

  MooseUtils::checkFileReadable(vector_file_name);

  fileslipsys.open(vector_file_name.c_str());

  for (unsigned int i = 0; i < number_dislocation_systems; ++i)
  {
    // Read the plane normal
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      if (!(fileslipsys >> vec[j]))
        mooseError("CrystalPlasticityUpdate Error: Premature end of file reading plane normal "
                   "vectors from the file ",
                   vector_file_name);

    // Normalize the vectors
    Real magnitude;
    magnitude = Utility::pow<2>(vec[0]) + Utility::pow<2>(vec[1]) + Utility::pow<2>(vec[2]);
    magnitude = std::sqrt(magnitude);

    for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      plane_normal_vector(i * LIBMESH_DIM + j) = vec[j] / magnitude;

    // Read the direction
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      if (!(fileslipsys >> vec[j]))
        mooseError("CrystalPlasticityUpdate Error: Premature end of file reading direction vectors "
                   "from the file ",
                   vector_file_name);

    // Normalize the vectors
    magnitude = Utility::pow<2>(vec[0]) + Utility::pow<2>(vec[1]) + Utility::pow<2>(vec[2]);
    magnitude = std::sqrt(magnitude);

    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      direction_vector(i * LIBMESH_DIM + j) = vec[j] / magnitude;

    // Check that the normalized vectors are orthonormal
    magnitude = 0.0;
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      magnitude += direction_vector(i * LIBMESH_DIM + j) * plane_normal_vector(i * LIBMESH_DIM + j);

    if (std::abs(magnitude) > 1.0e-8)
    {
      orthonormal_error = true;
      break;
    }
    if (orthonormal_error)
      break;
  }

  fileslipsys.close();
}

void
CrystalPlasticityUpdate::sortCrossSlipFamilies()
{
  if (_number_cross_slip_directions == 0)
  {
    _cross_slip_familes.resize(0);
    return;
  }

  // If cross slip does occur, then set up the system of vectors for the families
  _cross_slip_familes.resize(_number_cross_slip_directions);
  // and set the first index of each inner vector
  for (unsigned int i = 0; i < _number_cross_slip_directions; ++i)
    _cross_slip_familes[i].resize(1);

  // Sort the index of the slip system based vectors into separte families
  unsigned int family_counter = 1;
  _cross_slip_familes[0][0] = 0;

  for (unsigned int i = 1; i < _number_slip_systems; ++i)
  {
    for (unsigned int j = 0; j < family_counter; ++j)
    {
      // check to see if the slip system direction i matches any of the existing slip directions
      // First calculate the dot product
      Real dot_product = 0.0;
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        unsigned int check_family_index = _cross_slip_familes[j][0];
        dot_product += std::abs(_slip_direction(check_family_index * LIBMESH_DIM + k) -
                                _slip_direction(i * LIBMESH_DIM + k));
      }
      // Then check if the dot product is one, if yes, add to family and break
      if (MooseUtils::absoluteFuzzyEqual(dot_product, 0.0))
      {
        _cross_slip_familes[j].push_back(i);
        if (_cross_slip_familes[j].size() > _number_cross_slip_planes)
          mooseError(
              "Exceeded the number of cross slip planes allowed in a single cross slip family");

        break; // exit the loop over the exisiting cross slip families and move to the next slip
               // direction
      }
      // The slip system in question does not belong to an existing family
      else if (j == (family_counter - 1) && !MooseUtils::absoluteFuzzyEqual(dot_product, 0.0))
      {
        if (family_counter > _number_cross_slip_directions)
          mooseError("Exceeds the number of cross slip directions specified for this material");

        _cross_slip_familes[family_counter][0] = i;
        family_counter++;
        break;
      }
    }
  }

#ifdef DEBUG
  mooseWarning("Checking the slip system ordering now:");
  for (unsigned int i = 0; i < _number_cross_slip_directions; ++i)
  {
    Moose::out << "In cross slip family " << i << std::endl;
    for (unsigned int j = 0; j < _number_cross_slip_planes; ++j)
      Moose::out << " is the slip direction number " << _cross_slip_familes[i][j] << std::endl;
  }
#endif
}

unsigned int
CrystalPlasticityUpdate::indentifyCrossSlipFamily(const unsigned int index)
{
  for (unsigned int i = 0; i < _number_cross_slip_directions; ++i)
    for (unsigned int j = 0; j < _number_cross_slip_planes; ++j)
      if (_cross_slip_familes[i][j] == index)
        return i;

  // Should never reach this statement
  mooseError("The supplied slip system index is not among the slip system families sorted.");
}

void
CrystalPlasticityUpdate::setQp(unsigned int qp)
{
  _qp = qp;
}
