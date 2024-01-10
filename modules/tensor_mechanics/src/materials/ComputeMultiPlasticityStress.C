//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMultiPlasticityStress.h"
#include "MultiPlasticityDebugger.h"
#include "MatrixTools.h"

#include "MooseException.h"
#include "RotationMatrix.h" // for rotVecToZ

#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", ComputeMultiPlasticityStress);

InputParameters
ComputeMultiPlasticityStress::validParams()
{
  InputParameters params = ComputeStressBase::validParams();
  params += MultiPlasticityDebugger::validParams();
  params.addClassDescription("Base class for multi-surface finite-strain plasticity");
  params.addRangeCheckedParam<unsigned int>("max_NR_iterations",
                                            20,
                                            "max_NR_iterations>0",
                                            "Maximum number of Newton-Raphson iterations allowed");
  params.addRequiredParam<Real>("ep_plastic_tolerance",
                                "The Newton-Raphson process is only deemed "
                                "converged if the plastic strain increment "
                                "constraints have L2 norm less than this.");
  params.addRangeCheckedParam<Real>(
      "min_stepsize",
      0.01,
      "min_stepsize>0 & min_stepsize<=1",
      "If ordinary Newton-Raphson + line-search fails, then the applied strain increment is "
      "subdivided, and the return-map is tried again.  This parameter is the minimum fraction of "
      "applied strain increment that may be applied before the algorithm gives up entirely");
  params.addRangeCheckedParam<Real>("max_stepsize_for_dumb",
                                    0.01,
                                    "max_stepsize_for_dumb>0 & max_stepsize_for_dumb<=1",
                                    "If your deactivation_scheme is 'something_to_dumb', then "
                                    "'dumb' will only be used if the stepsize falls below this "
                                    "value.  This parameter is useful because the 'dumb' scheme is "
                                    "computationally expensive");
  MooseEnum deactivation_scheme("optimized safe dumb optimized_to_safe safe_to_dumb "
                                "optimized_to_safe_to_dumb optimized_to_dumb",
                                "optimized");
  params.addParam<MooseEnum>(
      "deactivation_scheme",
      deactivation_scheme,
      "Scheme by which constraints are deactivated.  (NOTE: This is irrelevant if there is only "
      "one yield surface.)  safe: return to the yield surface and then deactivate constraints with "
      "negative plasticity multipliers.  optimized: deactivate a constraint as soon as its "
      "plasticity multiplier becomes negative.  dumb: iteratively try all combinations of active "
      "constraints until the solution is found.  You may specify fall-back options.  Eg "
      "optimized_to_safe: first use 'optimized', and if that fails, try the return with 'safe'.");
  params.addParam<RealVectorValue>(
      "transverse_direction",
      "If this parameter is provided, before the return-map algorithm is "
      "called a rotation is performed so that the 'z' axis in the new "
      "frame lies along the transverse_direction in the original frame.  "
      "After returning, the inverse rotation is performed.  The "
      "transverse_direction will itself rotate with large strains.  This "
      "is so that transversely-isotropic plasticity models may be easily "
      "defined in the frame where the isotropy holds in the x-y plane.");
  params.addParam<bool>("ignore_failures",
                        false,
                        "The return-map algorithm will return with the best admissible "
                        "stresses and internal parameters that it can, even if they don't "
                        "fully correspond to the applied strain increment.  To speed "
                        "computations, this flag can be set to true, the max_NR_iterations "
                        "set small, and the min_stepsize large.");
  MooseEnum tangent_operator("elastic linear nonlinear", "nonlinear");
  params.addParam<MooseEnum>("tangent_operator",
                             tangent_operator,
                             "Type of tangent operator to return.  'elastic': return the "
                             "elasticity tensor.  'linear': return the consistent tangent operator "
                             "that is correct for plasticity with yield functions linear in "
                             "stress.  'nonlinear': return the full, general consistent tangent "
                             "operator.  The calculations assume the hardening potentials are "
                             "independent of stress and hardening parameters.");
  params.addParam<bool>("perform_finite_strain_rotations",
                        true,
                        "Tensors are correctly rotated in "
                        "finite-strain simulations.  For "
                        "optimal performance you can set "
                        "this to 'false' if you are only "
                        "ever using small strains");
  params.addClassDescription("Material for multi-surface finite-strain plasticity");
  return params;
}

ComputeMultiPlasticityStress::ComputeMultiPlasticityStress(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    MultiPlasticityDebugger(this),
    _max_iter(getParam<unsigned int>("max_NR_iterations")),
    _min_stepsize(getParam<Real>("min_stepsize")),
    _max_stepsize_for_dumb(getParam<Real>("max_stepsize_for_dumb")),
    _ignore_failures(getParam<bool>("ignore_failures")),

    _tangent_operator_type((TangentOperatorEnum)(int)getParam<MooseEnum>("tangent_operator")),

    _epp_tol(getParam<Real>("ep_plastic_tolerance")),

    _dummy_pm(0),

    _cumulative_pm(0),

    _deactivation_scheme((DeactivationSchemeEnum)(int)getParam<MooseEnum>("deactivation_scheme")),

    _n_supplied(parameters.isParamValid("transverse_direction")),
    _n_input(_n_supplied ? getParam<RealVectorValue>("transverse_direction") : RealVectorValue()),
    _rot(RealTensorValue()),

    _perform_finite_strain_rotations(getParam<bool>("perform_finite_strain_rotations")),

    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _plastic_strain(declareProperty<RankTwoTensor>("plastic_strain")),
    _plastic_strain_old(getMaterialPropertyOld<RankTwoTensor>("plastic_strain")),
    _intnl(declareProperty<std::vector<Real>>("plastic_internal_parameter")),
    _intnl_old(getMaterialPropertyOld<std::vector<Real>>("plastic_internal_parameter")),
    _yf(declareProperty<std::vector<Real>>("plastic_yield_function")),
    _iter(declareProperty<Real>("plastic_NR_iterations")), // this is really an unsigned int, but
                                                           // for visualisation i convert it to Real
    _linesearch_needed(declareProperty<Real>("plastic_linesearch_needed")), // this is really a
                                                                            // boolean, but for
                                                                            // visualisation i
                                                                            // convert it to Real
    _ld_encountered(declareProperty<Real>(
        "plastic_linear_dependence_encountered")), // this is really a boolean, but for
                                                   // visualisation i convert it to Real
    _constraints_added(declareProperty<Real>("plastic_constraints_added")), // this is really a
                                                                            // boolean, but for
                                                                            // visualisation i
                                                                            // convert it to Real
    _n(declareProperty<RealVectorValue>("plastic_transverse_direction")),
    _n_old(getMaterialPropertyOld<RealVectorValue>("plastic_transverse_direction")),

    _strain_increment(getMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _total_strain_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "total_strain")),
    _rotation_increment(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation_increment")),

    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "elastic_strain")),

    // TODO: This design does NOT work. It makes these materials construction order dependent and it
    // disregards block restrictions.
    _cosserat(hasMaterialProperty<RankTwoTensor>("curvature") &&
              hasMaterialProperty<RankFourTensor>("elastic_flexural_rigidity_tensor")),
    _curvature(_cosserat ? &getMaterialPropertyByName<RankTwoTensor>("curvature") : nullptr),
    _elastic_flexural_rigidity_tensor(
        _cosserat ? &getMaterialPropertyByName<RankFourTensor>("elastic_flexural_rigidity_tensor")
                  : nullptr),
    _couple_stress(_cosserat ? &declareProperty<RankTwoTensor>("couple_stress") : nullptr),
    _couple_stress_old(_cosserat ? &getMaterialPropertyOld<RankTwoTensor>("couple_stress")
                                 : nullptr),
    _Jacobian_mult_couple(_cosserat ? &declareProperty<RankFourTensor>("couple_Jacobian_mult")
                                    : nullptr),

    _my_elasticity_tensor(RankFourTensor()),
    _my_strain_increment(RankTwoTensor()),
    _my_flexural_rigidity_tensor(RankFourTensor()),
    _my_curvature(RankTwoTensor())
{
  if (_epp_tol <= 0)
    mooseError("ComputeMultiPlasticityStress: ep_plastic_tolerance must be positive");

  if (_n_supplied)
  {
    // normalise the inputted transverse_direction
    if (_n_input.norm() == 0)
      mooseError(
          "ComputeMultiPlasticityStress: transverse_direction vector must not have zero length");
    else
      _n_input /= _n_input.norm();
  }

  if (_num_surfaces == 1)
    _deactivation_scheme = safe;
}

void
ComputeMultiPlasticityStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();

  _plastic_strain[_qp].zero();

  _intnl[_qp].assign(_num_models, 0);

  _yf[_qp].assign(_num_surfaces, 0);

  _dummy_pm.assign(_num_surfaces, 0);

  _iter[_qp] = 0.0; // this is really an unsigned int, but for visualisation i convert it to Real
  _linesearch_needed[_qp] = 0;
  _ld_encountered[_qp] = 0;
  _constraints_added[_qp] = 0;

  _n[_qp] = _n_input;

  if (_cosserat)
    (*_couple_stress)[_qp].zero();

  if (_fspb_debug == "jacobian")
  {
    checkDerivatives();
    mooseError("Finite-differencing completed.  Exiting with no error");
  }
}

void
ComputeMultiPlasticityStress::computeQpStress()
{
  // the following "_my" variables can get rotated by preReturnMap and postReturnMap
  _my_elasticity_tensor = _elasticity_tensor[_qp];
  _my_strain_increment = _strain_increment[_qp];
  if (_cosserat)
  {
    _my_flexural_rigidity_tensor = (*_elastic_flexural_rigidity_tensor)[_qp];
    _my_curvature = (*_curvature)[_qp];
  }

  if (_fspb_debug == "jacobian_and_linear_system")
  {
    // cannot do this at initQpStatefulProperties level since E_ijkl is not defined
    checkJacobian(_elasticity_tensor[_qp].invSymm(), _intnl_old[_qp]);
    checkSolution(_elasticity_tensor[_qp].invSymm());
    mooseError("Finite-differencing completed.  Exiting with no error");
  }

  preReturnMap(); // do rotations to new frame if necessary

  unsigned int number_iterations;
  bool linesearch_needed = false;
  bool ld_encountered = false;
  bool constraints_added = false;

  _cumulative_pm.assign(_num_surfaces, 0);
  // try a "quick" return first - this can be purely elastic, or a customised plastic return defined
  // by a TensorMechanicsPlasticXXXX UserObject
  const bool found_solution = quickStep(rot(_stress_old[_qp]),
                                        _stress[_qp],
                                        _intnl_old[_qp],
                                        _intnl[_qp],
                                        _dummy_pm,
                                        _cumulative_pm,
                                        rot(_plastic_strain_old[_qp]),
                                        _plastic_strain[_qp],
                                        _my_elasticity_tensor,
                                        _my_strain_increment,
                                        _yf[_qp],
                                        number_iterations,
                                        _Jacobian_mult[_qp],
                                        computeQpStress_function,
                                        true);

  // if not purely elastic or the customised stuff failed, do some plastic return
  if (!found_solution)
    plasticStep(rot(_stress_old[_qp]),
                _stress[_qp],
                _intnl_old[_qp],
                _intnl[_qp],
                rot(_plastic_strain_old[_qp]),
                _plastic_strain[_qp],
                _my_elasticity_tensor,
                _my_strain_increment,
                _yf[_qp],
                number_iterations,
                linesearch_needed,
                ld_encountered,
                constraints_added,
                _Jacobian_mult[_qp]);

  if (_cosserat)
  {
    (*_couple_stress)[_qp] = (*_elastic_flexural_rigidity_tensor)[_qp] * _my_curvature;
    (*_Jacobian_mult_couple)[_qp] = _my_flexural_rigidity_tensor;
  }

  postReturnMap(); // rotate back from new frame if necessary

  _iter[_qp] = 1.0 * number_iterations;
  _linesearch_needed[_qp] = linesearch_needed;
  _ld_encountered[_qp] = ld_encountered;
  _constraints_added[_qp] = constraints_added;

  // Update measures of strain
  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _my_strain_increment -
                         (_plastic_strain[_qp] - _plastic_strain_old[_qp]);

  // Rotate the tensors to the current configuration
  if (_perform_finite_strain_rotations)
  {
    _stress[_qp] = _rotation_increment[_qp] * _stress[_qp] * _rotation_increment[_qp].transpose();
    _elastic_strain[_qp] =
        _rotation_increment[_qp] * _elastic_strain[_qp] * _rotation_increment[_qp].transpose();
    _plastic_strain[_qp] =
        _rotation_increment[_qp] * _plastic_strain[_qp] * _rotation_increment[_qp].transpose();
  }
}

RankTwoTensor
ComputeMultiPlasticityStress::rot(const RankTwoTensor & tens)
{
  if (!_n_supplied)
    return tens;
  return tens.rotated(_rot);
}

void
ComputeMultiPlasticityStress::preReturnMap()
{
  if (_n_supplied)
  {
    // this is a rotation matrix that will rotate _n to the "z" axis
    _rot = RotationMatrix::rotVecToZ(_n[_qp]);

    // rotate the tensors to this frame
    _my_elasticity_tensor.rotate(_rot);
    _my_strain_increment.rotate(_rot);
    if (_cosserat)
    {
      _my_flexural_rigidity_tensor.rotate(_rot);
      _my_curvature.rotate(_rot);
    }
  }
}

void
ComputeMultiPlasticityStress::postReturnMap()
{
  if (_n_supplied)
  {
    // this is a rotation matrix that will rotate "z" axis back to _n
    _rot = _rot.transpose();

    // rotate the tensors back to original frame where _n is correctly oriented
    _my_elasticity_tensor.rotate(_rot);
    _Jacobian_mult[_qp].rotate(_rot);
    _my_strain_increment.rotate(_rot);
    _stress[_qp].rotate(_rot);
    _plastic_strain[_qp].rotate(_rot);
    if (_cosserat)
    {
      _my_flexural_rigidity_tensor.rotate(_rot);
      (*_Jacobian_mult_couple)[_qp].rotate(_rot);
      _my_curvature.rotate(_rot);
      (*_couple_stress)[_qp].rotate(_rot);
    }

    // Rotate n by _rotation_increment
    for (const auto i : make_range(Moose::dim))
    {
      _n[_qp](i) = 0;
      for (const auto j : make_range(Moose::dim))
        _n[_qp](i) += _rotation_increment[_qp](i, j) * _n_old[_qp](j);
    }
  }
}

bool
ComputeMultiPlasticityStress::quickStep(const RankTwoTensor & stress_old,
                                        RankTwoTensor & stress,
                                        const std::vector<Real> & intnl_old,
                                        std::vector<Real> & intnl,
                                        std::vector<Real> & pm,
                                        std::vector<Real> & cumulative_pm,
                                        const RankTwoTensor & plastic_strain_old,
                                        RankTwoTensor & plastic_strain,
                                        const RankFourTensor & E_ijkl,
                                        const RankTwoTensor & strain_increment,
                                        std::vector<Real> & yf,
                                        unsigned int & iterations,
                                        RankFourTensor & consistent_tangent_operator,
                                        const quickStep_called_from_t called_from,
                                        bool final_step)
{
  iterations = 0;

  unsigned num_plastic_returns;
  RankTwoTensor delta_dp;

  // the following does the customized returnMap algorithm
  // for all the plastic models.
  unsigned custom_model = 0;
  bool successful_return = returnMapAll(stress_old + E_ijkl * strain_increment,
                                        intnl_old,
                                        E_ijkl,
                                        _epp_tol,
                                        stress,
                                        intnl,
                                        pm,
                                        cumulative_pm,
                                        delta_dp,
                                        yf,
                                        num_plastic_returns,
                                        custom_model);

  // the following updates the plastic_strain, when necessary
  // and calculates the consistent_tangent_operator, when necessary
  if (num_plastic_returns == 0)
  {
    // if successful_return = true, then a purely elastic step
    // if successful_return = false, then >=1 plastic model is in
    //    inadmissible zone and failed to return using its customized
    //    returnMap function.
    // In either case:
    plastic_strain = plastic_strain_old;
    if (successful_return && final_step)
    {
      if (called_from == computeQpStress_function)
        consistent_tangent_operator = E_ijkl;
      else // cannot necessarily use E_ijkl since different plastic models may have been active
           // during other substeps
        consistent_tangent_operator =
            consistentTangentOperator(stress, intnl, E_ijkl, pm, cumulative_pm);
    }
    return successful_return;
  }
  else if (num_plastic_returns == 1 && successful_return)
  {
    // one model has successfully completed its custom returnMap algorithm
    // and the other models have signalled they are elastic at
    // the trial stress
    plastic_strain = plastic_strain_old + delta_dp;
    if (final_step)
    {
      if (called_from == computeQpStress_function && _f[custom_model]->useCustomCTO())
      {
        if (_tangent_operator_type == elastic)
          consistent_tangent_operator = E_ijkl;
        else
        {
          std::vector<Real> custom_model_pm;
          for (unsigned surface = 0; surface < _f[custom_model]->numberSurfaces(); ++surface)
            custom_model_pm.push_back(cumulative_pm[_surfaces_given_model[custom_model][surface]]);
          consistent_tangent_operator =
              _f[custom_model]->consistentTangentOperator(stress_old + E_ijkl * strain_increment,
                                                          intnl_old[custom_model],
                                                          stress,
                                                          intnl[custom_model],
                                                          E_ijkl,
                                                          custom_model_pm);
        }
      }
      else // cannot necessarily use the custom consistentTangentOperator since different plastic
           // models may have been active during other substeps or the custom model says not to use
           // its custom CTO algorithm
        consistent_tangent_operator =
            consistentTangentOperator(stress, intnl, E_ijkl, pm, cumulative_pm);
    }
    return true;
  }
  else // presumably returnMapAll is incorrectly coded!
    mooseError("ComputeMultiPlasticityStress::quickStep   should not get here!");
}

bool
ComputeMultiPlasticityStress::plasticStep(const RankTwoTensor & stress_old,
                                          RankTwoTensor & stress,
                                          const std::vector<Real> & intnl_old,
                                          std::vector<Real> & intnl,
                                          const RankTwoTensor & plastic_strain_old,
                                          RankTwoTensor & plastic_strain,
                                          const RankFourTensor & E_ijkl,
                                          const RankTwoTensor & strain_increment,
                                          std::vector<Real> & yf,
                                          unsigned int & iterations,
                                          bool & linesearch_needed,
                                          bool & ld_encountered,
                                          bool & constraints_added,
                                          RankFourTensor & consistent_tangent_operator)
{
  /**
   * the idea in the following is to potentially subdivide the
   * strain increment into smaller fractions, of size "step_size".
   * First step_size = 1 is tried, and if that fails then 0.5 is
   * tried, then 0.25, etc.  As soon as a step is successful, its
   * results are put into the "good" variables, which are used
   * if a subsequent step fails.  If >= 2 consecutive steps are
   * successful, the step_size is increased by 1.2
   *
   * The point of all this is that I hope that the
   * time-step for the entire mesh need not be cut if there
   * are only a few "bad" quadpoints where the return-map
   * is difficult
   */

  bool return_successful = false;

  // total number of Newton-Raphson iterations used
  unsigned int iter = 0;

  Real step_size = 1.0;
  Real time_simulated = 0.0;

  // the "good" variables hold the latest admissible stress
  // and internal parameters.
  RankTwoTensor stress_good = stress_old;
  RankTwoTensor plastic_strain_good = plastic_strain_old;
  std::vector<Real> intnl_good(_num_models);
  for (unsigned model = 0; model < _num_models; ++model)
    intnl_good[model] = intnl_old[model];
  std::vector<Real> yf_good(_num_surfaces);

  // Following is necessary because I want strain_increment to be "const"
  // but I also want to be able to subdivide an initial_stress
  RankTwoTensor this_strain_increment = strain_increment;

  RankTwoTensor dep = step_size * this_strain_increment;

  _cumulative_pm.assign(_num_surfaces, 0);

  unsigned int num_consecutive_successes = 0;
  while (time_simulated < 1.0 && step_size >= _min_stepsize)
  {
    iter = 0;
    return_successful = returnMap(stress_good,
                                  stress,
                                  intnl_good,
                                  intnl,
                                  plastic_strain_good,
                                  plastic_strain,
                                  E_ijkl,
                                  dep,
                                  yf,
                                  iter,
                                  step_size <= _max_stepsize_for_dumb,
                                  linesearch_needed,
                                  ld_encountered,
                                  constraints_added,
                                  time_simulated + step_size >= 1,
                                  consistent_tangent_operator,
                                  _cumulative_pm);
    iterations += iter;

    if (return_successful)
    {
      num_consecutive_successes += 1;
      time_simulated += step_size;

      if (time_simulated < 1.0) // this condition is just for optimization: if time_simulated=1 then
                                // the "good" quantities are no longer needed
      {
        stress_good = stress;
        plastic_strain_good = plastic_strain;
        for (unsigned model = 0; model < _num_models; ++model)
          intnl_good[model] = intnl[model];
        for (unsigned surface = 0; surface < _num_surfaces; ++surface)
          yf_good[surface] = yf[surface];
        if (num_consecutive_successes >= 2)
          step_size *= 1.2;
      }
      step_size = std::min(step_size, 1.0 - time_simulated); // avoid overshoots
    }
    else
    {
      step_size *= 0.5;
      num_consecutive_successes = 0;
      stress = stress_good;
      plastic_strain = plastic_strain_good;
      for (unsigned model = 0; model < _num_models; ++model)
        intnl[model] = intnl_good[model];
      yf.resize(_num_surfaces); // might have excited with junk
      for (unsigned surface = 0; surface < _num_surfaces; ++surface)
        yf[surface] = yf_good[surface];
      dep = step_size * this_strain_increment;
    }
  }

  if (!return_successful)
  {
    if (_ignore_failures)
    {
      stress = stress_good;
      plastic_strain = plastic_strain_good;
      for (unsigned model = 0; model < _num_models; ++model)
        intnl[model] = intnl_good[model];
      for (unsigned surface = 0; surface < _num_surfaces; ++surface)
        yf[surface] = yf_good[surface];
    }
    else
    {
      Moose::out << "After reducing the stepsize to " << step_size
                 << " with original strain increment with L2norm " << this_strain_increment.L2norm()
                 << " the returnMap algorithm failed" << std::endl;

      _fspb_debug_stress = stress_good + E_ijkl * dep;
      _fspb_debug_pm.assign(
          _num_surfaces,
          1); // this is chosen arbitrarily - please change if a more suitable value occurs to you!
      _fspb_debug_intnl.resize(_num_models);
      for (unsigned model = 0; model < _num_models; ++model)
        _fspb_debug_intnl[model] = intnl_good[model];
      checkDerivatives();
      checkJacobian(_my_elasticity_tensor.invSymm(), _intnl_old[_qp]);
      checkSolution(_my_elasticity_tensor.invSymm());
      mooseError("Exiting\n");
    }
  }

  return return_successful;
}

bool
ComputeMultiPlasticityStress::returnMap(const RankTwoTensor & stress_old,
                                        RankTwoTensor & stress,
                                        const std::vector<Real> & intnl_old,
                                        std::vector<Real> & intnl,
                                        const RankTwoTensor & plastic_strain_old,
                                        RankTwoTensor & plastic_strain,
                                        const RankFourTensor & E_ijkl,
                                        const RankTwoTensor & strain_increment,
                                        std::vector<Real> & f,
                                        unsigned int & iter,
                                        bool can_revert_to_dumb,
                                        bool & linesearch_needed,
                                        bool & ld_encountered,
                                        bool & constraints_added,
                                        bool final_step,
                                        RankFourTensor & consistent_tangent_operator,
                                        std::vector<Real> & cumulative_pm)
{

  // The "consistency parameters" (plastic multipliers)
  // Change in plastic strain in this timestep = pm*flowPotential
  // Each pm must be non-negative
  std::vector<Real> pm;
  pm.assign(_num_surfaces, 0.0);

  bool successful_return = quickStep(stress_old,
                                     stress,
                                     intnl_old,
                                     intnl,
                                     pm,
                                     cumulative_pm,
                                     plastic_strain_old,
                                     plastic_strain,
                                     E_ijkl,
                                     strain_increment,
                                     f,
                                     iter,
                                     consistent_tangent_operator,
                                     returnMap_function,
                                     final_step);

  if (successful_return)
    return successful_return;

  // Here we know that the trial stress and intnl_old
  // is inadmissible, and we have to return from those values
  // value to the yield surface.  There are three
  // types of constraints we have to satisfy, listed
  // below, and calculated in calculateConstraints(...)
  // f<=0, epp=0, ic=0 (up to tolerances), and these are
  //     guaranteed to hold if nr_res2<=0.5
  //
  // Kuhn-Tucker conditions must also be satisfied
  // These are:
  // pm>=0, which may not hold upon exit of the NR loops
  //     due to _deactivation_scheme!=optimized;
  // pm*f=0 (up to tolerances), which may not hold upon exit
  //     of the NR loops if a constraint got deactivated
  //     due to linear dependence, and then f<0, and its pm>0

  // Plastic strain constraint, L2 norm must be zero (up to a tolerance)
  RankTwoTensor epp;

  // Yield function constraint passed to this function as
  // std::vector<Real> & f
  // Each yield function must be <= 0 (up to tolerance)
  // Note that only the constraints that are active will be
  // contained in f until the final few lines of returnMap

  // Internal constraint(s), must be zero (up to a tolerance)
  // Note that only the constraints that are active will be
  // contained in ic.
  std::vector<Real> ic;

  // Record the stress before Newton-Raphson in case of failure-and-restart
  RankTwoTensor initial_stress = stress;

  iter = 0;

  // Initialize the set of active constraints
  // At this stage, the active constraints are
  // those that exceed their _f_tol
  // active constraints.
  std::vector<bool> act;
  buildActiveConstraints(f, stress, intnl, E_ijkl, act);

  // Inverse of E_ijkl (assuming symmetric)
  RankFourTensor E_inv = E_ijkl.invSymm();

  // convenience variable that holds the change in plastic strain incurred during the return
  // delta_dp = plastic_strain - plastic_strain_old
  // delta_dp = E^{-1}*(initial_stress - stress), where initial_stress = E*(strain -
  // plastic_strain_old)
  RankTwoTensor delta_dp = RankTwoTensor();

  // whether single step was successful (whether line search was successful, and whether turning off
  // constraints was successful)
  bool single_step_success = true;

  // deactivation scheme
  DeactivationSchemeEnum deact_scheme = _deactivation_scheme;

  // For complicated deactivation schemes we have to record the initial active set
  std::vector<bool> initial_act;
  initial_act.resize(_num_surfaces);
  if (_deactivation_scheme == optimized_to_safe ||
      _deactivation_scheme == optimized_to_safe_to_dumb ||
      _deactivation_scheme == optimized_to_dumb)
  {
    // if "optimized" fails we can change the deactivation scheme to "safe", etc
    deact_scheme = optimized;
    for (unsigned surface = 0; surface < _num_surfaces; ++surface)
      initial_act[surface] = act[surface];
  }

  if (_deactivation_scheme == safe_to_dumb)
    deact_scheme = safe;

  // For "dumb" deactivation, the active set takes all combinations until a solution is found
  int dumb_iteration = 0;
  std::vector<unsigned int> dumb_order;

  if (_deactivation_scheme == dumb ||
      (_deactivation_scheme == optimized_to_safe_to_dumb && can_revert_to_dumb) ||
      (_deactivation_scheme == safe_to_dumb && can_revert_to_dumb) ||
      (_deactivation_scheme == optimized_to_dumb && can_revert_to_dumb))
    buildDumbOrder(stress, intnl, dumb_order);

  if (_deactivation_scheme == dumb)
  {
    incrementDumb(dumb_iteration, dumb_order, act);
    yieldFunction(stress, intnl, act, f);
  }

  // To avoid any re-trials of "act" combinations that
  // we've already tried and rejected, i record the
  // combinations in actives_tried
  std::set<unsigned int> actives_tried;
  actives_tried.insert(activeCombinationNumber(act));

  // The residual-squared that the line-search will reduce
  // Later it will get contributions from epp and ic, but
  // at present these are zero
  Real nr_res2 = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (act[surface])
      nr_res2 += 0.5 * Utility::pow<2>(f[surface] / _f[modelNumber(surface)]->_f_tol);

  successful_return = false;

  bool still_finding_solution = true;
  while (still_finding_solution)
  {
    single_step_success = true;
    unsigned int local_iter = 0;

    // The Newton-Raphson loops
    while (nr_res2 > 0.5 && local_iter++ < _max_iter && single_step_success)
      single_step_success = singleStep(nr_res2,
                                       stress,
                                       intnl_old,
                                       intnl,
                                       pm,
                                       delta_dp,
                                       E_inv,
                                       f,
                                       epp,
                                       ic,
                                       act,
                                       deact_scheme,
                                       linesearch_needed,
                                       ld_encountered);

    bool nr_good = (nr_res2 <= 0.5 && local_iter <= _max_iter && single_step_success);

    iter += local_iter;

    // 'act' might have changed due to using deact_scheme = optimized, so
    actives_tried.insert(activeCombinationNumber(act));

    if (!nr_good)
    {
      // failure of NR routine.
      // We might be able to change the deactivation_scheme and
      // then re-try the NR starting from the initial values
      // Or, if deact_scheme == "dumb", we just increarse the
      // dumb_iteration number and re-try
      bool change_scheme = false;
      bool increment_dumb = false;
      change_scheme = canChangeScheme(deact_scheme, can_revert_to_dumb);
      if (!change_scheme && deact_scheme == dumb)
        increment_dumb = canIncrementDumb(dumb_iteration);

      still_finding_solution = (change_scheme || increment_dumb);

      if (change_scheme)
        changeScheme(initial_act,
                     can_revert_to_dumb,
                     initial_stress,
                     intnl_old,
                     deact_scheme,
                     act,
                     dumb_iteration,
                     dumb_order);

      if (increment_dumb)
        incrementDumb(dumb_iteration, dumb_order, act);

      if (!still_finding_solution)
      {
        // we cannot change the scheme, or have run out of "dumb" options
        successful_return = false;
        break;
      }
    }

    bool kt_good = false;
    if (nr_good)
    {
      // check Kuhn-Tucker
      kt_good = checkKuhnTucker(f, pm, act);
      if (!kt_good)
      {
        if (deact_scheme != dumb)
        {
          applyKuhnTucker(f, pm, act);

          // true if we haven't tried this active set before
          still_finding_solution =
              (actives_tried.find(activeCombinationNumber(act)) == actives_tried.end());
          if (!still_finding_solution)
          {
            // must have tried turning off the constraints already.
            // so try changing the scheme
            if (canChangeScheme(deact_scheme, can_revert_to_dumb))
            {
              still_finding_solution = true;
              changeScheme(initial_act,
                           can_revert_to_dumb,
                           initial_stress,
                           intnl_old,
                           deact_scheme,
                           act,
                           dumb_iteration,
                           dumb_order);
            }
          }
        }
        else
        {
          bool increment_dumb = false;
          increment_dumb = canIncrementDumb(dumb_iteration);
          still_finding_solution = increment_dumb;
          if (increment_dumb)
            incrementDumb(dumb_iteration, dumb_order, act);
        }

        if (!still_finding_solution)
        {
          // have tried turning off the constraints already,
          // or have run out of "dumb" options
          successful_return = false;
          break;
        }
      }
    }

    bool admissible = false;
    if (nr_good && kt_good)
    {
      // check admissible
      std::vector<Real> all_f;
      if (_num_surfaces == 1)
        admissible = true; // for a single surface if NR has exited successfully then (stress,
                           // intnl) must be admissible
      else
        admissible = checkAdmissible(stress, intnl, all_f);

      if (!admissible)
      {
        // Not admissible.
        // We can try adding constraints back in
        // We can try changing the deactivation scheme
        // Or, if deact_scheme == dumb, just increase dumb_iteration
        bool add_constraints = canAddConstraints(act, all_f);
        if (add_constraints)
        {
          constraints_added = true;
          std::vector<bool> act_plus(_num_surfaces,
                                     false); // "act" with the positive constraints added in
          for (unsigned surface = 0; surface < _num_surfaces; ++surface)
            if (act[surface] ||
                (!act[surface] && (all_f[surface] > _f[modelNumber(surface)]->_f_tol)))
              act_plus[surface] = true;
          if (actives_tried.find(activeCombinationNumber(act_plus)) == actives_tried.end())
          {
            // haven't tried this combination of actives yet
            constraints_added = true;
            for (unsigned surface = 0; surface < _num_surfaces; ++surface)
              act[surface] = act_plus[surface];
          }
          else
            add_constraints = false; // haven't managed to add a new combination
        }

        bool change_scheme = false;
        bool increment_dumb = false;

        if (!add_constraints)
          change_scheme = canChangeScheme(deact_scheme, can_revert_to_dumb);

        if (!add_constraints && !change_scheme && deact_scheme == dumb)
          increment_dumb = canIncrementDumb(dumb_iteration);

        still_finding_solution = (add_constraints || change_scheme || increment_dumb);

        if (change_scheme)
          changeScheme(initial_act,
                       can_revert_to_dumb,
                       initial_stress,
                       intnl_old,
                       deact_scheme,
                       act,
                       dumb_iteration,
                       dumb_order);

        if (increment_dumb)
          incrementDumb(dumb_iteration, dumb_order, act);

        if (!still_finding_solution)
        {
          // we cannot change the scheme, or have run out of "dumb" options
          successful_return = false;
          break;
        }
      }
    }

    successful_return = (nr_good && admissible && kt_good);
    if (successful_return)
      break;

    if (still_finding_solution)
    {
      stress = initial_stress;
      delta_dp = RankTwoTensor(); // back to zero change in plastic strain
      for (unsigned model = 0; model < _num_models; ++model)
        intnl[model] = intnl_old[model]; // back to old internal params
      pm.assign(_num_surfaces, 0.0);     // back to zero plastic multipliers

      unsigned num_active = numberActive(act);
      if (num_active == 0)
      {
        successful_return = false;
        break; // failure
      }

      actives_tried.insert(activeCombinationNumber(act));

      // Since "act" set has changed, either by changing deact_scheme, or by KT failing, so need to
      // re-calculate nr_res2
      yieldFunction(stress, intnl, act, f);

      nr_res2 = 0;
      unsigned ind = 0;
      for (unsigned surface = 0; surface < _num_surfaces; ++surface)
        if (act[surface])
        {
          if (f[ind] > _f[modelNumber(surface)]->_f_tol)
            nr_res2 += 0.5 * Utility::pow<2>(f[ind] / _f[modelNumber(surface)]->_f_tol);
          ind++;
        }
    }
  }

  // returned, with either success or failure
  if (successful_return)
  {
    plastic_strain += delta_dp;

    for (unsigned surface = 0; surface < _num_surfaces; ++surface)
      cumulative_pm[surface] += pm[surface];

    if (final_step)
      consistent_tangent_operator =
          consistentTangentOperator(stress, intnl, E_ijkl, pm, cumulative_pm);

    if (f.size() != _num_surfaces)
    {
      // at this stage f.size() = num_active, but we need to return with all the yield functions
      // evaluated, so:
      act.assign(_num_surfaces, true);
      yieldFunction(stress, intnl, act, f);
    }
  }

  return successful_return;
}

bool
ComputeMultiPlasticityStress::canAddConstraints(const std::vector<bool> & act,
                                                const std::vector<Real> & all_f)
{
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (!act[surface] && (all_f[surface] > _f[modelNumber(surface)]->_f_tol))
      return true;
  return false;
}

bool
ComputeMultiPlasticityStress::canChangeScheme(DeactivationSchemeEnum current_deactivation_scheme,
                                              bool can_revert_to_dumb)
{
  if (current_deactivation_scheme == optimized && _deactivation_scheme == optimized_to_safe)
    return true;

  if (current_deactivation_scheme == optimized && _deactivation_scheme == optimized_to_safe_to_dumb)
    return true;

  if (current_deactivation_scheme == safe && _deactivation_scheme == safe_to_dumb &&
      can_revert_to_dumb)
    return true;

  if (current_deactivation_scheme == safe && _deactivation_scheme == optimized_to_safe_to_dumb &&
      can_revert_to_dumb)
    return true;

  if (current_deactivation_scheme == optimized && _deactivation_scheme == optimized_to_dumb &&
      can_revert_to_dumb)
    return true;

  return false;
}

void
ComputeMultiPlasticityStress::changeScheme(const std::vector<bool> & initial_act,
                                           bool can_revert_to_dumb,
                                           const RankTwoTensor & initial_stress,
                                           const std::vector<Real> & intnl_old,
                                           DeactivationSchemeEnum & current_deactivation_scheme,
                                           std::vector<bool> & act,
                                           int & dumb_iteration,
                                           std::vector<unsigned int> & dumb_order)
{
  if (current_deactivation_scheme == optimized &&
      (_deactivation_scheme == optimized_to_safe ||
       _deactivation_scheme == optimized_to_safe_to_dumb))
  {
    current_deactivation_scheme = safe;
    for (unsigned surface = 0; surface < _num_surfaces; ++surface)
      act[surface] = initial_act[surface];
  }
  else if ((current_deactivation_scheme == safe &&
            (_deactivation_scheme == optimized_to_safe_to_dumb ||
             _deactivation_scheme == safe_to_dumb) &&
            can_revert_to_dumb) ||
           (current_deactivation_scheme == optimized && _deactivation_scheme == optimized_to_dumb &&
            can_revert_to_dumb))
  {
    current_deactivation_scheme = dumb;
    dumb_iteration = 0;
    buildDumbOrder(initial_stress, intnl_old, dumb_order);
    incrementDumb(dumb_iteration, dumb_order, act);
  }
}

bool
ComputeMultiPlasticityStress::singleStep(Real & nr_res2,
                                         RankTwoTensor & stress,
                                         const std::vector<Real> & intnl_old,
                                         std::vector<Real> & intnl,
                                         std::vector<Real> & pm,
                                         RankTwoTensor & delta_dp,
                                         const RankFourTensor & E_inv,
                                         std::vector<Real> & f,
                                         RankTwoTensor & epp,
                                         std::vector<Real> & ic,
                                         std::vector<bool> & active,
                                         DeactivationSchemeEnum deactivation_scheme,
                                         bool & linesearch_needed,
                                         bool & ld_encountered)
{
  bool successful_step; // return value

  Real nr_res2_before_step = nr_res2;
  RankTwoTensor stress_before_step;
  std::vector<Real> intnl_before_step;
  std::vector<Real> pm_before_step;
  RankTwoTensor delta_dp_before_step;

  if (deactivation_scheme == optimized)
  {
    // we potentially use the "before_step" quantities, so record them here
    stress_before_step = stress;
    intnl_before_step.resize(_num_models);
    for (unsigned model = 0; model < _num_models; ++model)
      intnl_before_step[model] = intnl[model];
    pm_before_step.resize(_num_surfaces);
    for (unsigned surface = 0; surface < _num_surfaces; ++surface)
      pm_before_step[surface] = pm[surface];
    delta_dp_before_step = delta_dp;
  }

  // During the Newton-Raphson procedure, we'll be
  // changing the following parameters in order to
  // (attempt to) satisfy the constraints.
  RankTwoTensor dstress; // change in stress
  std::vector<Real> dpm; // change in plasticity multipliers ("consistency parameters").  For ALL
                         // contraints (active and deactive)
  std::vector<Real>
      dintnl; // change in internal parameters.  For ALL internal params (active and deactive)

  // The constraints that have been deactivated for this NR step
  // due to the flow directions being linearly dependent
  std::vector<bool> deact_ld;
  deact_ld.assign(_num_surfaces, false);

  /* After NR and linesearch, if _deactivation_scheme == "optimized", the
   * active plasticity multipliers are checked for non-negativity.  If some
   * are negative then they are deactivated forever, and the NR step is
   * re-done starting from the _before_step quantities recorded above
   */
  bool constraints_changing = true;
  bool reinstated_actives;
  while (constraints_changing)
  {
    // calculate dstress, dpm and dintnl for one full Newton-Raphson step
    nrStep(stress, intnl_old, intnl, pm, E_inv, delta_dp, dstress, dpm, dintnl, active, deact_ld);

    for (unsigned surface = 0; surface < deact_ld.size(); ++surface)
      if (deact_ld[surface])
      {
        ld_encountered = true;
        break;
      }

    // perform a line search
    // The line-search will exit with updated values
    successful_step = lineSearch(nr_res2,
                                 stress,
                                 intnl_old,
                                 intnl,
                                 pm,
                                 E_inv,
                                 delta_dp,
                                 dstress,
                                 dpm,
                                 dintnl,
                                 f,
                                 epp,
                                 ic,
                                 active,
                                 deact_ld,
                                 linesearch_needed);

    if (!successful_step)
      // completely bomb out
      return successful_step;

    // See if any active constraints need to be removed, and the step re-done
    constraints_changing = false;
    if (deactivation_scheme == optimized)
    {
      for (unsigned surface = 0; surface < _num_surfaces; ++surface)
        if (active[surface] && pm[surface] < 0.0)
          constraints_changing = true;
    }

    if (constraints_changing)
    {
      stress = stress_before_step;
      delta_dp = delta_dp_before_step;
      nr_res2 = nr_res2_before_step;
      for (unsigned surface = 0; surface < _num_surfaces; ++surface)
      {
        if (active[surface] && pm[surface] < 0.0)
        {
          // turn off the constraint forever
          active[surface] = false;
          pm_before_step[surface] = 0.0;
          intnl_before_step[modelNumber(surface)] =
              intnl_old[modelNumber(surface)]; // don't want to muck-up hardening!
        }
        intnl[modelNumber(surface)] = intnl_before_step[modelNumber(surface)];
        pm[surface] = pm_before_step[surface];
      }
      if (numberActive(active) == 0)
      {
        // completely bomb out
        successful_step = false;
        return successful_step;
      }
    }

    // reinstate any active values that have been turned off due to linear-dependence
    reinstated_actives = reinstateLinearDependentConstraints(deact_ld);
  } // ends "constraints_changing" loop

  // if active constraints were reinstated then nr_res2 needs to be re-calculated so it is correct
  // upson returning
  if (reinstated_actives)
  {
    bool completely_converged = true;
    if (successful_step && nr_res2 < 0.5)
    {
      // Here we have converged to the correct solution if
      // all the yield functions are < 0.  Excellent!
      //
      // This is quite tricky - perhaps i can refactor to make it more obvious.
      //
      // Because actives are now reinstated, the residual2
      // calculation below will give nr_res2 > 0.5, because it won't
      // realise that we only had to set the active-but-not-deactivated f = 0,
      // and not the entire active set.  If we pass that nr_res2 back from
      // this function then the calling function will not realise we've converged!
      // Therefore, check for this case
      unsigned ind = 0;
      for (unsigned surface = 0; surface < _num_surfaces; ++surface)
        if (active[surface])
          if (f[ind++] > _f[modelNumber(surface)]->_f_tol)
            completely_converged = false;
    }
    else
      completely_converged = false;

    if (!completely_converged)
      nr_res2 = residual2(pm, f, epp, ic, active, deact_ld);
  }

  return successful_step;
}

bool
ComputeMultiPlasticityStress::reinstateLinearDependentConstraints(
    std::vector<bool> & deactivated_due_to_ld)
{
  bool reinstated_actives = false;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (deactivated_due_to_ld[surface])
      reinstated_actives = true;

  deactivated_due_to_ld.assign(_num_surfaces, false);
  return reinstated_actives;
}

unsigned
ComputeMultiPlasticityStress::numberActive(const std::vector<bool> & active)
{
  unsigned num_active = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active[surface])
      num_active++;
  return num_active;
}

bool
ComputeMultiPlasticityStress::checkAdmissible(const RankTwoTensor & stress,
                                              const std::vector<Real> & intnl,
                                              std::vector<Real> & all_f)
{
  std::vector<bool> act;
  act.assign(_num_surfaces, true);

  yieldFunction(stress, intnl, act, all_f);

  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (all_f[surface] > _f[modelNumber(surface)]->_f_tol)
      return false;

  return true;
}

bool
ComputeMultiPlasticityStress::checkKuhnTucker(const std::vector<Real> & f,
                                              const std::vector<Real> & pm,
                                              const std::vector<bool> & active)
{
  unsigned ind = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    if (active[surface])
    {
      if (f[ind++] < -_f[modelNumber(surface)]->_f_tol)
        if (pm[surface] != 0)
          return false;
    }
    else if (pm[surface] != 0)
      mooseError("Crash due to plastic multiplier not being zero.  This occurred because of poor "
                 "coding!!");
  }
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (pm[surface] < 0)
      return false;

  return true;
}

void
ComputeMultiPlasticityStress::applyKuhnTucker(const std::vector<Real> & f,
                                              const std::vector<Real> & pm,
                                              std::vector<bool> & active)
{
  bool turned_off = false;
  unsigned ind = 0;

  // turn off all active surfaces that have f<0 and pm!=0
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    if (active[surface])
    {
      if (f[ind++] < -_f[modelNumber(surface)]->_f_tol)
        if (pm[surface] != 0)
        {
          turned_off = true;
          active[surface] = false;
        }
    }
    else if (pm[surface] != 0)
      mooseError("Crash due to plastic multiplier not being zero.  This occurred because of poor "
                 "coding!!");
  }

  // if didn't turn off anything yet, turn off surface with minimum pm
  if (!turned_off)
  {
    int surface_to_turn_off = -1;
    Real min_pm = 0;
    for (unsigned surface = 0; surface < _num_surfaces; ++surface)
      if (pm[surface] < min_pm)
      {
        min_pm = pm[surface];
        surface_to_turn_off = surface;
      }
    if (surface_to_turn_off >= 0)
      active[surface_to_turn_off] = false;
  }
}

Real
ComputeMultiPlasticityStress::residual2(const std::vector<Real> & pm,
                                        const std::vector<Real> & f,
                                        const RankTwoTensor & epp,
                                        const std::vector<Real> & ic,
                                        const std::vector<bool> & active,
                                        const std::vector<bool> & deactivated_due_to_ld)
{
  Real nr_res2 = 0;
  unsigned ind = 0;

  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active[surface])
    {
      if (!deactivated_due_to_ld[surface])
      {
        if (!(pm[surface] == 0 && f[ind] <= 0))
          nr_res2 += 0.5 * Utility::pow<2>(f[ind] / _f[modelNumber(surface)]->_f_tol);
      }
      else if (deactivated_due_to_ld[surface] && f[ind] > 0)
        nr_res2 += 0.5 * Utility::pow<2>(f[ind] / _f[modelNumber(surface)]->_f_tol);
      ind++;
    }

  nr_res2 += 0.5 * Utility::pow<2>(epp.L2norm() / _epp_tol);

  std::vector<bool> active_not_deact(_num_surfaces);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    active_not_deact[surface] = (active[surface] && !deactivated_due_to_ld[surface]);
  ind = 0;
  for (unsigned model = 0; model < _num_models; ++model)
    if (anyActiveSurfaces(model, active_not_deact))
      nr_res2 += 0.5 * Utility::pow<2>(ic[ind++] / _f[model]->_ic_tol);

  return nr_res2;
}

bool
ComputeMultiPlasticityStress::lineSearch(Real & nr_res2,
                                         RankTwoTensor & stress,
                                         const std::vector<Real> & intnl_old,
                                         std::vector<Real> & intnl,
                                         std::vector<Real> & pm,
                                         const RankFourTensor & E_inv,
                                         RankTwoTensor & delta_dp,
                                         const RankTwoTensor & dstress,
                                         const std::vector<Real> & dpm,
                                         const std::vector<Real> & dintnl,
                                         std::vector<Real> & f,
                                         RankTwoTensor & epp,
                                         std::vector<Real> & ic,
                                         const std::vector<bool> & active,
                                         const std::vector<bool> & deactivated_due_to_ld,
                                         bool & linesearch_needed)
{
  // Line search algorithm straight out of "Numerical Recipes"

  bool success =
      true; // return value: will be false if linesearch couldn't reduce the residual-squared

  // Aim is to decrease residual2

  Real lam = 1.0; // the line-search parameter: 1.0 is a full Newton step
  Real lam_min =
      1E-10; // minimum value of lam allowed - perhaps this should be dynamically calculated?
  Real f0 = nr_res2;         // initial value of residual2
  Real slope = -2 * nr_res2; // "Numerical Recipes" uses -b*A*x, in order to check for roundoff, but
                             // i hope the nrStep would warn if there were problems.
  Real tmp_lam;              // cached value of lam used in quadratic & cubic line search
  Real f2 = nr_res2;         // cached value of f = residual2 used in the cubic in the line search
  Real lam2 = lam;           // cached value of lam used in the cubic in the line search

  // pm during the line-search
  std::vector<Real> ls_pm;
  ls_pm.resize(pm.size());

  // delta_dp during the line-search
  RankTwoTensor ls_delta_dp;

  // internal parameter during the line-search
  std::vector<Real> ls_intnl;
  ls_intnl.resize(intnl.size());

  // stress during the line-search
  RankTwoTensor ls_stress;

  // flow directions (not used in line search, but calculateConstraints returns this parameter)
  std::vector<RankTwoTensor> r;

  while (true)
  {
    // update the variables using this line-search parameter
    for (unsigned alpha = 0; alpha < pm.size(); ++alpha)
      ls_pm[alpha] = pm[alpha] + dpm[alpha] * lam;
    ls_delta_dp = delta_dp - E_inv * dstress * lam;
    for (unsigned a = 0; a < intnl.size(); ++a)
      ls_intnl[a] = intnl[a] + dintnl[a] * lam;
    ls_stress = stress + dstress * lam;

    // calculate the new active yield functions, epp and active internal constraints
    calculateConstraints(ls_stress, intnl_old, ls_intnl, ls_pm, ls_delta_dp, f, r, epp, ic, active);

    // calculate the new residual-squared
    nr_res2 = residual2(ls_pm, f, epp, ic, active, deactivated_due_to_ld);

    if (nr_res2 < f0 + 1E-4 * lam * slope)
      break;
    else if (lam < lam_min)
    {
      success = false;
      // restore plastic multipliers, yield functions, etc to original values
      for (unsigned alpha = 0; alpha < pm.size(); ++alpha)
        ls_pm[alpha] = pm[alpha];
      ls_delta_dp = delta_dp;
      for (unsigned a = 0; a < intnl.size(); ++a)
        ls_intnl[a] = intnl[a];
      ls_stress = stress;
      calculateConstraints(
          ls_stress, intnl_old, ls_intnl, ls_pm, ls_delta_dp, f, r, epp, ic, active);
      nr_res2 = residual2(ls_pm, f, epp, ic, active, deactivated_due_to_ld);
      break;
    }
    else if (lam == 1.0)
    {
      // model as a quadratic
      tmp_lam = -slope / 2.0 / (nr_res2 - f0 - slope);
    }
    else
    {
      // model as a cubic
      Real rhs1 = nr_res2 - f0 - lam * slope;
      Real rhs2 = f2 - f0 - lam2 * slope;
      Real a = (rhs1 / Utility::pow<2>(lam) - rhs2 / Utility::pow<2>(lam2)) / (lam - lam2);
      Real b =
          (-lam2 * rhs1 / Utility::pow<2>(lam) + lam * rhs2 / Utility::pow<2>(lam2)) / (lam - lam2);
      if (a == 0)
        tmp_lam = -slope / (2.0 * b);
      else
      {
        Real disc = Utility::pow<2>(b) - 3 * a * slope;
        if (disc < 0)
          tmp_lam = 0.5 * lam;
        else if (b <= 0)
          tmp_lam = (-b + std::sqrt(disc)) / (3.0 * a);
        else
          tmp_lam = -slope / (b + std::sqrt(disc));
      }
      if (tmp_lam > 0.5 * lam)
        tmp_lam = 0.5 * lam;
    }
    lam2 = lam;
    f2 = nr_res2;
    lam = std::max(tmp_lam, 0.1 * lam);
  }

  if (lam < 1.0)
    linesearch_needed = true;

  // assign the quantities found in the line-search
  // back to the originals
  for (unsigned alpha = 0; alpha < pm.size(); ++alpha)
    pm[alpha] = ls_pm[alpha];
  delta_dp = ls_delta_dp;
  for (unsigned a = 0; a < intnl.size(); ++a)
    intnl[a] = ls_intnl[a];
  stress = ls_stress;

  return success;
}

void
ComputeMultiPlasticityStress::buildDumbOrder(const RankTwoTensor & stress,
                                             const std::vector<Real> & intnl,
                                             std::vector<unsigned int> & dumb_order)
{
  if (dumb_order.size() != 0)
    return;

  std::vector<bool> act;
  act.assign(_num_surfaces, true);

  std::vector<Real> f;
  yieldFunction(stress, intnl, act, f);
  std::vector<RankTwoTensor> df_dstress;
  dyieldFunction_dstress(stress, intnl, act, df_dstress);

  typedef std::pair<Real, unsigned> pair_for_sorting;
  std::vector<pair_for_sorting> dist(_num_surfaces);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    dist[surface].first = f[surface] / df_dstress[surface].L2norm();
    dist[surface].second = surface;
  }
  std::sort(dist.begin(), dist.end()); // sorted in ascending order of f/df_dstress

  dumb_order.resize(_num_surfaces);
  for (unsigned i = 0; i < _num_surfaces; ++i)
    dumb_order[i] = dist[_num_surfaces - 1 - i].second;
  // now dumb_order[0] is the surface with the greatest f/df_dstress
}

void
ComputeMultiPlasticityStress::incrementDumb(int & dumb_iteration,
                                            const std::vector<unsigned int> & dumb_order,
                                            std::vector<bool> & act)
{
  dumb_iteration += 1;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    act[dumb_order[surface]] =
        (dumb_iteration &
         (1 << surface)); // returns true if the surface_th bit of dumb_iteration == 1
}

bool
ComputeMultiPlasticityStress::canIncrementDumb(int dumb_iteration)
{
  // (1 << _num_surfaces) = 2^_num_surfaces
  return ((dumb_iteration + 1) < (1 << _num_surfaces));
}

unsigned int
ComputeMultiPlasticityStress::activeCombinationNumber(const std::vector<bool> & act)
{
  unsigned num = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (act[surface])
      num += (1 << surface); // (1 << x) = 2^x

  return num;
}

RankFourTensor
ComputeMultiPlasticityStress::consistentTangentOperator(const RankTwoTensor & stress,
                                                        const std::vector<Real> & intnl,
                                                        const RankFourTensor & E_ijkl,
                                                        const std::vector<Real> & pm_this_step,
                                                        const std::vector<Real> & cumulative_pm)
{

  if (_tangent_operator_type == elastic)
    return E_ijkl;

  // Typically act_at_some_step = act, but it is possible
  // that when subdividing a strain increment, a surface
  // is only active for one sub-step
  std::vector<bool> act_at_some_step(_num_surfaces);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    act_at_some_step[surface] = (cumulative_pm[surface] > 0);

  // "act" might contain surfaces that are linearly dependent
  // with others.  Only the plastic multipliers that are > 0
  // for this strain increment need to be varied to find
  // the consistent tangent operator
  std::vector<bool> act_vary(_num_surfaces);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    act_vary[surface] = (pm_this_step[surface] > 0);

  std::vector<RankTwoTensor> df_dstress;
  dyieldFunction_dstress(stress, intnl, act_vary, df_dstress);
  std::vector<Real> df_dintnl;
  dyieldFunction_dintnl(stress, intnl, act_vary, df_dintnl);
  std::vector<RankTwoTensor> r;
  flowPotential(stress, intnl, act_vary, r);
  std::vector<RankFourTensor> dr_dstress_at_some_step;
  dflowPotential_dstress(stress, intnl, act_at_some_step, dr_dstress_at_some_step);
  std::vector<RankTwoTensor> dr_dintnl_at_some_step;
  dflowPotential_dintnl(stress, intnl, act_at_some_step, dr_dintnl_at_some_step);
  std::vector<Real> h;
  hardPotential(stress, intnl, act_vary, h);

  unsigned ind1;
  unsigned ind2;

  // r_minus_stuff[alpha] = r[alpha] -
  // pm_cumulatve[gamma]*dr[gamma]_dintnl[a]_at_some_step*h[a][alpha], with alpha only being in
  // act_vary, but gamma being act_at_some_step
  std::vector<RankTwoTensor> r_minus_stuff;
  ind1 = 0;
  for (unsigned surface1 = 0; surface1 < _num_surfaces; ++surface1)
    if (act_vary[surface1])
    {
      r_minus_stuff.push_back(r[ind1]);
      ind2 = 0;
      for (unsigned surface2 = 0; surface2 < _num_surfaces; ++surface2)
        if (act_at_some_step[surface2])
        {
          if (modelNumber(surface1) == modelNumber(surface2))
          {
            r_minus_stuff.back() -=
                cumulative_pm[surface2] * dr_dintnl_at_some_step[ind2] * h[ind1];
          }
          ind2++;
        }
      ind1++;
    }

  unsigned int num_currently_active = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (act_vary[surface])
      num_currently_active += 1;

  // zzz is a matrix in the form that can be easily
  // inverted by MatrixTools::inverse
  // Eg for num_currently_active = 3
  // (zzz[0] zzz[1] zzz[2])
  // (zzz[3] zzz[4] zzz[5])
  // (zzz[6] zzz[7] zzz[8])
  std::vector<PetscScalar> zzz;
  zzz.assign(num_currently_active * num_currently_active, 0.0);

  ind1 = 0;
  RankTwoTensor r2;
  for (unsigned surface1 = 0; surface1 < _num_surfaces; ++surface1)
    if (act_vary[surface1])
    {
      ind2 = 0;
      for (unsigned surface2 = 0; surface2 < _num_surfaces; ++surface2)
        if (act_vary[surface2])
        {
          r2 = df_dstress[ind1] * (E_ijkl * r_minus_stuff[ind2]);
          zzz[ind1 * num_currently_active + ind2] += r2(0, 0) + r2(1, 1) + r2(2, 2);
          if (modelNumber(surface1) == modelNumber(surface2))
            zzz[ind1 * num_currently_active + ind2] += df_dintnl[ind1] * h[ind2];
          ind2++;
        }
      ind1++;
    }

  if (num_currently_active > 0)
  {
    // invert zzz, in place.  if num_currently_active = 0 then zzz is not needed.
    try
    {
      MatrixTools::inverse(zzz, num_currently_active);
    }
    catch (const MooseException & e)
    {
      // in the very rare case of zzz being singular, just return the "elastic" tangent operator
      return E_ijkl;
    }
  }

  RankFourTensor strain_coeff = E_ijkl;
  ind1 = 0;
  for (unsigned surface1 = 0; surface1 < _num_surfaces; ++surface1)
    if (act_vary[surface1])
    {
      RankTwoTensor part1 = E_ijkl * r_minus_stuff[ind1];
      ind2 = 0;
      for (unsigned surface2 = 0; surface2 < _num_surfaces; ++surface2)
        if (act_vary[surface2])
        {
          RankTwoTensor part2 = E_ijkl * df_dstress[ind2];
          for (unsigned i = 0; i < 3; i++)
            for (unsigned j = 0; j < 3; j++)
              for (unsigned k = 0; k < 3; k++)
                for (unsigned l = 0; l < 3; l++)
                  strain_coeff(i, j, k, l) -=
                      part1(i, j) * part2(k, l) * zzz[ind1 * num_currently_active + ind2];
          ind2++;
        }
      ind1++;
    }

  if (_tangent_operator_type == linear)
    return strain_coeff;

  RankFourTensor stress_coeff(RankFourTensor::initIdentitySymmetricFour);

  RankFourTensor part3;
  ind1 = 0;
  for (unsigned surface1 = 0; surface1 < _num_surfaces; ++surface1)
    if (act_at_some_step[surface1])
    {
      part3 += cumulative_pm[surface1] * E_ijkl * dr_dstress_at_some_step[ind1];
      ind1++;
    }

  stress_coeff += part3;

  part3 = part3.transposeMajor(); // this is because below i want df_dstress[ind2]*part3, and this
                                  // equals (part3.transposeMajor())*df_dstress[ind2]

  ind1 = 0;
  for (unsigned surface1 = 0; surface1 < _num_surfaces; ++surface1)
    if (act_vary[surface1])
    {
      RankTwoTensor part1 = E_ijkl * r_minus_stuff[ind1];
      ind2 = 0;
      for (unsigned surface2 = 0; surface2 < _num_surfaces; ++surface2)
        if (act_vary[surface2])
        {
          RankTwoTensor part2 = part3 * df_dstress[ind2];
          for (unsigned i = 0; i < 3; i++)
            for (unsigned j = 0; j < 3; j++)
              for (unsigned k = 0; k < 3; k++)
                for (unsigned l = 0; l < 3; l++)
                  stress_coeff(i, j, k, l) -=
                      part1(i, j) * part2(k, l) * zzz[ind1 * num_currently_active + ind2];
          ind2++;
        }
      ind1++;
    }

  // need to find the inverse of stress_coeff, but remember
  // stress_coeff does not have the symmetries commonly found
  // in tensor mechanics:
  // stress_coeff(i, j, k, l) = stress_coeff(j, i, k, l) = stress_coeff(i, j, l, k) !=
  // stress_coeff(k, l, i, j)
  // (note the final not-equals).  We want s_inv, such that
  // s_inv(i, j, m, n)*stress_coeff(m, n, k, l) = (de_ik*de_jl + de_il*de_jk)/2
  // where de_ij = 1 if i=j, and 0 otherwise.
  RankFourTensor s_inv;
  try
  {
    s_inv = stress_coeff.invSymm();
  }
  catch (const MooseException & e)
  {
    return strain_coeff; // when stress_coeff is singular (perhaps for incompressible plasticity?)
                         // return the "linear" tangent operator
  }

  return s_inv * strain_coeff;
}
