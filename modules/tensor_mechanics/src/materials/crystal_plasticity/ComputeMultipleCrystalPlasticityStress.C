//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMultipleCrystalPlasticityStress.h"

#include "CrystalPlasticityStressUpdateBase.h"
#include "libmesh/utility.h"
#include "Conversion.h"
#include "MooseException.h"

registerMooseObject("TensorMechanicsApp", ComputeMultipleCrystalPlasticityStress);

InputParameters
ComputeMultipleCrystalPlasticityStress::validParams()
{
  InputParameters params = ComputeFiniteStrainElasticStress::validParams();

  params.addClassDescription(
      "Crystal Plasticity base class: handles the Newton iteration over the stress residual and "
      "calculates the Jacobian based on constitutive laws from multiple material classes "
      "that are inherited from CrystalPlasticityStressUpdateBase");
  params.addParam<std::string>(
      "base_name",
      "Optional parameter that allows the user to define multiple mechanics material systems on "
      "the same block, i.e. for multiple phases");

  params.addRequiredParam<std::vector<MaterialName>>(
      "crystal_plasticity_models",
      "The material objects to use to calculate crystal plasticity stress and strains.");

  params.addParam<MooseEnum>("tan_mod_type",
                             MooseEnum("exact none", "none"),
                             "Type of tangent moduli for preconditioner: default elastic");
  params.addParam<Real>("rtol", 1e-6, "Constitutive stress residual relative tolerance");
  params.addParam<Real>("abs_tol", 1e-6, "Constitutive stress residual absolute tolerance");
  params.addParam<unsigned int>("maxiter", 100, "Maximum number of iterations for stress update");
  params.addParam<unsigned int>(
      "maxiter_state_variable", 100, "Maximum number of iterations for state variable update");
  params.addParam<unsigned int>(
      "maximum_substep_iteration", 1, "Maximum number of substep iteration");
  params.addParam<bool>("use_line_search", false, "Use line search in constitutive update");
  params.addParam<Real>("min_line_search_step_size", 0.01, "Minimum line search step size");
  params.addParam<Real>("line_search_tol", 0.5, "Line search bisection method tolerance");
  params.addParam<unsigned int>(
      "line_search_maxiter", 20, "Line search bisection method maximum number of iteration");
  params.addParam<MooseEnum>("line_search_method",
                             MooseEnum("CUT_HALF BISECTION", "CUT_HALF"),
                             "The method used in line search");

  return params;
}

ComputeMultipleCrystalPlasticityStress::ComputeMultipleCrystalPlasticityStress(
    const InputParameters & parameters)
  : ComputeFiniteStrainElasticStress(parameters),
    _num_models(getParam<std::vector<MaterialName>>("crystal_plasticity_models").size()),

    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor")),

    _rtol(getParam<Real>("rtol")),
    _abs_tol(getParam<Real>("abs_tol")),
    _maxiter(getParam<unsigned int>("maxiter")),
    _maxiterg(getParam<unsigned int>("maxiter_state_variable")),
    _tan_mod_type(getParam<MooseEnum>("tan_mod_type").getEnum<TangentModuliType>()),
    _max_substep_iter(getParam<unsigned int>("maximum_substep_iteration")),
    _use_line_search(getParam<bool>("use_line_search")),
    _min_line_search_step_size(getParam<Real>("min_line_search_step_size")),
    _line_search_tolerance(getParam<Real>("line_search_tol")),
    _line_search_max_iterations(getParam<unsigned int>("line_search_maxiter")),
    _line_search_method(getParam<MooseEnum>("line_search_method").getEnum<LineSearchMethod>()),
    _plastic_deformation_gradient(declareProperty<RankTwoTensor>("fp")),
    _plastic_deformation_gradient_old(getMaterialPropertyOld<RankTwoTensor>("fp")),
    _deformation_gradient(getMaterialProperty<RankTwoTensor>("deformation_gradient")),
    _deformation_gradient_old(getMaterialPropertyOld<RankTwoTensor>("deformation_gradient")),
    _pk2(declareProperty<RankTwoTensor>("pk2")), // 2nd Piola Kirchoff Stress
    _pk2_old(getMaterialPropertyOld<RankTwoTensor>("pk2")),
    _total_lagrangian_strain(
        declareProperty<RankTwoTensor>("total_lagrangian_strain")), // Lagrangian strain
    _update_rotation(declareProperty<RankTwoTensor>("update_rot")),
    _crysrot(getMaterialProperty<RankTwoTensor>(
        "crysrot")) // defined in the elasticity tensor classes for crystal plasticity
{
  _convergence_failed = false;
}

void
ComputeMultipleCrystalPlasticityStress::initQpStatefulProperties()
{
  _plastic_deformation_gradient[_qp].zero();
  _plastic_deformation_gradient[_qp].addIa(1.0);

  _pk2[_qp].zero();

  _total_lagrangian_strain[_qp].zero();

  _update_rotation[_qp].zero();
  _update_rotation[_qp].addIa(1.0);

  for (unsigned int i = 0; i < _num_models; ++i)
  {
    _models[i]->setQp(_qp);
    _models[i]->initQpStatefulProperties();
  }
}

void
ComputeMultipleCrystalPlasticityStress::initialSetup()
{
  std::vector<MaterialName> model_names =
      getParam<std::vector<MaterialName>>("crystal_plasticity_models");

  for (unsigned int i = 0; i < _num_models; ++i)
  {
    CrystalPlasticityStressUpdateBase * model =
        dynamic_cast<CrystalPlasticityStressUpdateBase *>(&getMaterialByName(model_names[i]));

    if (model)
    {
      _models.push_back(model);
      // TODO: check to make sure that the material model is compatible with this class
    }
    else
      mooseError("Model " + model_names[i] +
                 " is not compatible with ComputeMultipleCrystalPlasticityStress");
  }
}

void
ComputeMultipleCrystalPlasticityStress::computeQpStress()
{
  for (unsigned int i = 0; i < _num_models; ++i)
    _models[i]->setQp(_qp);

  updateStress(_stress[_qp], _Jacobian_mult[_qp]); // This is NOT the exact jacobian
}

void
ComputeMultipleCrystalPlasticityStress::updateStress(RankTwoTensor & cauchy_stress,
                                                     RankFourTensor & jacobian_mult)
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

  // Loop through all models and calculate the schmid tensor for the current state of the crystal
  // lattice
  for (unsigned int i = 0; i < _num_models; ++i)
    _models[i]->calculateFlowDirection(_crysrot[_qp]);

  do
  {
    _convergence_failed = false;
    preSolveQp();

    for (unsigned int i = 0; i < _num_models; ++i)
      _models[i]->setSubstepDt(_dt / num_substep);

    for (unsigned int istep = 0; istep < num_substep; ++istep)
    {
      _temporary_deformation_gradient =
          (static_cast<Real>(istep) + 1) / num_substep * _delta_deformation_gradient;
      _temporary_deformation_gradient += _temporary_deformation_gradient_old;

      solveQp();

      if (_convergence_failed)
      {
        substep_iter++;
        num_substep *= 2;
        break;
      }
    }

    if (substep_iter > _max_substep_iter && _convergence_failed)
      mooseException("ComputeMultipleCrystalPlasticityStress: Constitutive failure");
  } while (_convergence_failed);

  postSolveQp(cauchy_stress, jacobian_mult);
}

void
ComputeMultipleCrystalPlasticityStress::preSolveQp()
{
  for (unsigned int i = 0; i < _num_models; ++i)
    _models[i]->setInitialConstitutiveVariableValues();

  _pk2[_qp] = _pk2_old[_qp];
  _inverse_plastic_deformation_grad_old = _plastic_deformation_gradient_old[_qp].inverse();
}

void
ComputeMultipleCrystalPlasticityStress::solveQp()
{
  for (unsigned int i = 0; i < _num_models; ++i)
  {
    _models[i]->setSubstepConstitutiveVariableValues();
    _models[i]->calculateSlipResistance();
  }

  _inverse_plastic_deformation_grad = _inverse_plastic_deformation_grad_old;

  solveStateVariables();
  if (_convergence_failed)
    return; // pop back up and take a smaller substep

  for (unsigned int i = 0; i < _num_models; ++i)
    _models[i]->updateSubstepConstitutiveVariableValues();

  // save off the old F^{p} inverse now that have converged on the stress and state variables
  _inverse_plastic_deformation_grad_old = _inverse_plastic_deformation_grad;
}

void
ComputeMultipleCrystalPlasticityStress::postSolveQp(RankTwoTensor & cauchy_stress,
                                                    RankFourTensor & jacobian_mult)
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
ComputeMultipleCrystalPlasticityStress::solveStateVariables()
{
  unsigned int iteration;
  bool iter_flag = true;

  iteration = 0;
  // Check for slip system resistance update tolerance
  do
  {
    solveStress();
    if (_convergence_failed)
      return;

    _plastic_deformation_gradient[_qp] =
        _inverse_plastic_deformation_grad.inverse(); // the postSoveStress

    // Update slip system resistance and state variable after the stress has been finalized
    // We loop through all the models for each calculation
    // in order to make sure that when coupling appears, the state variables are updated based on
    // the same components
    for (unsigned int i = 0; i < _num_models; ++i)
      _models[i]->cacheStateVariablesBeforeUpdate();

    for (unsigned int i = 0; i < _num_models; ++i)
      _models[i]->calculateStateVariableEvolutionRateComponent();

    for (unsigned int i = 0; i < _num_models; ++i)
      if (!_models[i]->updateStateVariables())
        _convergence_failed = true;

    for (unsigned int i = 0; i < _num_models; ++i)
      _models[i]->calculateSlipResistance();

    if (_convergence_failed)
      return;

    for (unsigned int i = 0; i < _num_models; ++i)
    {
      // iter_flag = false, stop iteration if all values are converged and good to go
      // iter_flag = true, continue iteration if any value is not converged
      if (!_models[i]->areConstitutiveStateVariablesConverged())
      {
        iter_flag = true;
        break;
      }
      else
        iter_flag = false; // iter_flag = false, stop iteration only when all models returns true
    }

    if (iter_flag)
    {
#ifdef DEBUG
      mooseWarning("ComputeMultipleCrystalPlasticityStress: State variables (or the system "
                   "resistance) did not "
                   "converge at element ",
                   _current_elem->id(),
                   " and qp ",
                   _qp,
                   "\n");
#endif
    }
    iteration++;
  } while (iter_flag && iteration < _maxiterg);

  if (iteration == _maxiterg)
  {
#ifdef DEBUG
    mooseWarning("ComputeMultipleCrystalPlasticityStress: Hardness Integration error. Reached the "
                 "maximum number "
                 "of iterations to solve for the state variables at element ",
                 _current_elem->id(),
                 " and qp ",
                 _qp,
                 "\n");
#endif
    _convergence_failed = true;
  }
}

void
ComputeMultipleCrystalPlasticityStress::solveStress()
{
  unsigned int iteration = 0;
  RankTwoTensor dpk2;
  Real rnorm, rnorm0, rnorm_prev;

  // Calculate stress residual
  calculateResidualAndJacobian();
  if (_convergence_failed)
  {
#ifdef DEBUG
    mooseWarning("ComputeMultipleCrystalPlasticityStress: Slip increment exceeds tolerance - "
                 "Element number ",
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

    if (_convergence_failed)
    {
#ifdef DEBUG
      mooseWarning("ComputeMultipleCrystalPlasticityStress: Slip increment exceeds tolerance - "
                   "Element number ",
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
      mooseWarning("ComputeMultipleCrystalPlasticityStress: Failed with line search");
#endif
      _convergence_failed = true;
      return;
    }

    if (_use_line_search)
      rnorm = _residual_tensor.L2norm();

    iteration++;
  }

  if (iteration >= _maxiter)
  {
#ifdef DEBUG
    mooseWarning("ComputeMultipleCrystalPlasticityStress: Stress Integration error rmax = ",
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
    _convergence_failed = true;
  }
}

// Calculates stress residual equation and jacobian
void
ComputeMultipleCrystalPlasticityStress::calculateResidualAndJacobian()
{
  calculateResidual();
  if (_convergence_failed)
    return;
  calculateJacobian();
}

void
ComputeMultipleCrystalPlasticityStress::calculateResidual()
{
  RankTwoTensor ce, elastic_strain, ce_pk2, equivalent_slip_increment_per_model,
      equivalent_slip_increment, pk2_new;

  equivalent_slip_increment.zero();

  for (unsigned int i = 0; i < _num_models; ++i)
  {
    equivalent_slip_increment_per_model.zero();
    _models[i]->calculateShearStress(_pk2[_qp]);

    _convergence_failed = !_models[i]->calculateSlipRate();

    if (_convergence_failed)
      return;

    _models[i]->calculateEquivalentSlipIncrement(equivalent_slip_increment_per_model);
    equivalent_slip_increment += equivalent_slip_increment_per_model;
  }

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
ComputeMultipleCrystalPlasticityStress::calculateJacobian()
{
  // may not need to cache the dfpinvdpk2 here. need to double check
  RankFourTensor dfedfpinv, deedfe, dfpinvdpk2, dfpinvdpk2_per_model;

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

  for (unsigned int i = 0; i < _num_models; ++i)
  {
    _models[i]->calculateTotalPlasticDeformationGradientDerivative(
        dfpinvdpk2_per_model, _inverse_plastic_deformation_grad_old);
    dfpinvdpk2 += dfpinvdpk2_per_model;
  }

  _jacobian =
      RankFourTensor::IdentityFour() - (_elasticity_tensor[_qp] * deedfe * dfedfpinv * dfpinvdpk2);
}

void
ComputeMultipleCrystalPlasticityStress::calcTangentModuli(RankFourTensor & jacobian_mult)
{
  switch (_tan_mod_type)
  {
    case TangentModuliType::EXACT:
      elastoPlasticTangentModuli(jacobian_mult);
      break;
    default:
      elasticTangentModuli(jacobian_mult);
  }
}

void
ComputeMultipleCrystalPlasticityStress::elastoPlasticTangentModuli(RankFourTensor & jacobian_mult)
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

  auto je = _elastic_deformation_gradient.det();
  if (je > 0.0)
    tan_mod /= je;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
        dfedf(i, j, i, l) = _inverse_plastic_deformation_grad(l, j);

  jacobian_mult = tan_mod * dfedf;
}

void
ComputeMultipleCrystalPlasticityStress::elasticTangentModuli(RankFourTensor & jacobian_mult)
{
  // update jacobian_mult
  jacobian_mult = _elasticity_tensor[_qp];
}

bool
ComputeMultipleCrystalPlasticityStress::lineSearchUpdate(const Real & rnorm_prev,
                                                         const RankTwoTensor & dpk2)
{
  if (_line_search_method == LineSearchMethod::CutHalf)
  {
    Real rnorm;
    Real step = 1.0;

    do
    {
      _pk2[_qp] = _pk2[_qp] - step * dpk2;
      step /= 2.0;
      _pk2[_qp] = _pk2[_qp] + step * dpk2;

      calculateResidual();
      rnorm = _residual_tensor.L2norm();
    } while (rnorm > rnorm_prev && step > _min_line_search_step_size);

    // has norm improved or is the step still above minumum search step size?
    return (rnorm <= rnorm_prev || step > _min_line_search_step_size);
  }
  else if (_line_search_method == LineSearchMethod::Bisection)
  {
    unsigned int count = 0;
    Real step_a = 0.0;
    Real step_b = 1.0;
    Real step = 1.0;
    Real s_m = 1000.0;
    Real rnorm = 1000.0;

    calculateResidual();
    auto s_b = _residual_tensor.doubleContraction(dpk2);
    auto rnorm1 = _residual_tensor.L2norm();
    _pk2[_qp] = _pk2[_qp] - dpk2;
    calculateResidual();
    auto s_a = _residual_tensor.doubleContraction(dpk2);
    auto rnorm0 = _residual_tensor.L2norm();
    _pk2[_qp] = _pk2[_qp] + dpk2;

    if ((rnorm1 / rnorm0) < _line_search_tolerance || s_a * s_b > 0)
    {
      calculateResidual();
      return true;
    }

    while ((rnorm / rnorm0) > _line_search_tolerance && count < _line_search_max_iterations)
    {
      _pk2[_qp] = _pk2[_qp] - step * dpk2;
      step = 0.5 * (step_b + step_a);
      _pk2[_qp] = _pk2[_qp] + step * dpk2;
      calculateResidual();
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
  else
    mooseError("Line search method is not provided.");
}
