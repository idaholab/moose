/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PLC_LSH.h"

#include "SymmIsotropicElasticityTensor.h"

template <>
InputParameters
validParams<PLC_LSH>()
{
  InputParameters params = validParams<SolidModel>();

  // Power-law creep material parameters
  params.addRequiredParam<Real>("coefficient", "Leading coefficent in power-law equation");
  params.addRequiredParam<Real>("n_exponent", "Exponent on effective stress in power-law equation");
  params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
  params.addRequiredParam<Real>("activation_energy", "Activation energy");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");

  // Linear strain hardening parameters
  params.addRequiredParam<Real>("yield_stress",
                                "The point at which plastic strain begins accumulating");
  params.addRequiredParam<Real>("hardening_constant", "Hardening slope");

  // Sub-Newton Iteration control parameters
  params.addParam<unsigned int>("max_its", 30, "Maximum number of sub-newton iterations");
  params.addParam<bool>(
      "output_iteration_info", false, "Set true to output sub-newton iteration information");
  params.addParam<Real>(
      "relative_tolerance", 1e-5, "Relative convergence tolerance for sub-newtion iteration");
  params.addParam<Real>(
      "absolute_tolerance", 1e-20, "Absolute convergence tolerance for sub-newtion iteration");
  params.addParam<PostprocessorName>(
      "output", "", "The reporting postprocessor to use for the max_iterations value.");

  // Control of combined plasticity-creep iterarion
  params.addParam<Real>("absolute_stress_tolerance",
                        1e-5,
                        "Convergence tolerance for combined plasticity-creep stress iteration");

  return params;
}

PLC_LSH::PLC_LSH(const InputParameters & parameters)
  : SolidModel(parameters),
    _coefficient(parameters.get<Real>("coefficient")),
    _n_exponent(parameters.get<Real>("n_exponent")),
    _m_exponent(parameters.get<Real>("m_exponent")),
    _activation_energy(parameters.get<Real>("activation_energy")),
    _gas_constant(parameters.get<Real>("gas_constant")),

    _yield_stress(parameters.get<Real>("yield_stress")),
    _hardening_constant(parameters.get<Real>("hardening_constant")),

    _max_its(parameters.get<unsigned int>("max_its")),
    _output_iteration_info(getParam<bool>("output_iteration_info")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),

    _absolute_stress_tolerance(parameters.get<Real>("absolute_stress_tolerance")),

    _creep_strain(declareProperty<SymmTensor>("creep_strain")),
    _creep_strain_old(declarePropertyOld<SymmTensor>("creep_strain")),

    _plastic_strain(declareProperty<SymmTensor>("plastic_strain")),
    _plastic_strain_old(declarePropertyOld<SymmTensor>("plastic_strain")),

    _hardening_variable(declareProperty<Real>("hardening_variable")),
    _hardening_variable_old(declarePropertyOld<Real>("hardening_variable")),

    _output(getParam<PostprocessorName>("output") != "" ? &getPostprocessorValue("output") : NULL)

{
  if (_yield_stress <= 0)
  {
    mooseError("Yield stress must be greater than zero");
  }
}

void
PLC_LSH::initQpStatefulProperties()
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp] = 0;
  SolidModel::initQpStatefulProperties();
}

void
PLC_LSH::computeStress()
{
  // Given the stretching, compute the stress increment and add it to the old stress. Also update
  // the creep strain
  // stress = stressOld + stressIncrement
  // creep_strain = creep_strainOld + creep_strainIncrement

  if (_t_step == 0 && !_app.isRestarting())
    return;

  if (_output_iteration_info == true)
  {
    _console << std::endl
             << "iteration output for combined creep-plasticity solve:"
             << " time=" << _t << " temperature=" << _temperature[_qp] << " int_pt=" << _qp
             << std::endl;
  }

  // compute trial stress
  SymmTensor stress_new(*elasticityTensor() * _strain_increment);
  stress_new += _stress_old;

  SymmTensor creep_strain_increment;
  SymmTensor plastic_strain_increment;
  SymmTensor elastic_strain_increment;
  SymmTensor stress_new_last(stress_new);
  Real delS(_absolute_stress_tolerance + 1);
  Real first_delS(delS);
  unsigned int counter(0);

  while (counter < _max_its && delS > _absolute_stress_tolerance &&
         (delS / first_delS) > _relative_tolerance)
  {
    elastic_strain_increment = _strain_increment;
    elastic_strain_increment -= plastic_strain_increment;
    stress_new = *elasticityTensor() * elastic_strain_increment;
    stress_new += _stress_old;

    elastic_strain_increment = _strain_increment;
    computeCreep(elastic_strain_increment, creep_strain_increment, stress_new);

    // now use stress_new to calculate a new effective_trial_stress and determine if
    // yield has occured and if so, calculate the corresponding plastic strain

    elastic_strain_increment -= creep_strain_increment;

    computeLSH(elastic_strain_increment, plastic_strain_increment, stress_new);

    elastic_strain_increment -= plastic_strain_increment;

    // now check convergence
    SymmTensor deltaS(stress_new_last - stress_new);
    delS = std::sqrt(deltaS.doubleContraction(deltaS));
    if (counter == 0)
    {
      first_delS = delS;
    }
    stress_new_last = stress_new;

    if (_output_iteration_info == true)
    {
      _console << "stress_it=" << counter << " rel_delS=" << delS / first_delS
               << " rel_tol=" << _relative_tolerance << " abs_delS=" << delS
               << " abs_tol=" << _absolute_stress_tolerance << std::endl;
    }

    ++counter;
  }

  if (counter == _max_its && delS > _absolute_stress_tolerance &&
      (delS / first_delS) > _relative_tolerance)
  {
    mooseError("Max stress iteration hit during plasticity-creep solve!");
  }

  _strain_increment = elastic_strain_increment;
  _stress[_qp] = stress_new;
}

void
PLC_LSH::computeCreep(const SymmTensor & strain_increment,
                      SymmTensor & creep_strain_increment,
                      SymmTensor & stress_new)
{
  // compute deviatoric trial stress
  SymmTensor dev_trial_stress(stress_new);
  dev_trial_stress.addDiag(-dev_trial_stress.trace() / 3.0);

  // compute effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

  // Use Newton sub-iteration to determine effective creep strain increment

  Real exponential(1);
  if (_has_temp)
  {
    exponential = std::exp(-_activation_energy / (_gas_constant * _temperature[_qp]));
  }
  Real expTime = std::pow(_t, _m_exponent);

  Real del_p = 0;
  unsigned int it = 0;
  Real creep_residual = 10;
  Real norm_creep_residual = 10;
  Real first_norm_creep_residual = 10;

  while (it < _max_its && norm_creep_residual > _absolute_tolerance &&
         (norm_creep_residual / first_norm_creep_residual) > _relative_tolerance)
  {

    Real phi = _coefficient *
               std::pow(effective_trial_stress - 3 * _shear_modulus * del_p, _n_exponent) *
               exponential * expTime;
    Real dphi_ddelp =
        -3 * _coefficient * _shear_modulus * _n_exponent *
        std::pow(effective_trial_stress - 3 * _shear_modulus * del_p, _n_exponent - 1) *
        exponential * expTime;

    creep_residual = phi - del_p / _dt;
    norm_creep_residual = std::abs(creep_residual);
    if (it == 0)
    {
      first_norm_creep_residual = norm_creep_residual;
    }

    del_p += creep_residual / (1 / _dt - dphi_ddelp);

    if (_output_iteration_info == true)
    {
      _console << "crp_it=" << it << " trl_strs=" << effective_trial_stress << " phi=" << phi
               << " dphi=" << dphi_ddelp << " del_p=" << del_p
               << " rel_res=" << norm_creep_residual / first_norm_creep_residual
               << " rel_tol=" << _relative_tolerance << " abs_res=" << norm_creep_residual
               << " abs_tol=" << _absolute_tolerance << std::endl;
    }

    ++it;
  }

  if (it == _max_its && norm_creep_residual > _absolute_tolerance &&
      (norm_creep_residual / first_norm_creep_residual) > _relative_tolerance)
  {
    mooseError("Max sub-newton iteration hit during creep solve!");
  }

  // compute creep and elastic strain increments (avoid potential divide by zero - how should this
  // be done)?
  if (effective_trial_stress < 0.01)
  {
    effective_trial_stress = 0.01;
  }

  creep_strain_increment = dev_trial_stress;
  creep_strain_increment *= (1.5 * del_p / effective_trial_stress);

  SymmTensor elastic_strain_increment(strain_increment);
  elastic_strain_increment -= creep_strain_increment;

  // compute stress increment
  stress_new = *elasticityTensor() * elastic_strain_increment;

  // update stress and creep strain
  stress_new += _stress_old;

  _creep_strain[_qp] = creep_strain_increment;
  _creep_strain[_qp] += _creep_strain_old[_qp];
}

void
PLC_LSH::computeLSH(const SymmTensor & strain_increment,
                    SymmTensor & plastic_strain_increment,
                    SymmTensor & stress_new)
{

  // compute deviatoric trial stress
  SymmTensor dev_trial_stress(stress_new);
  dev_trial_stress.addDiag(-stress_new.trace() / 3);

  // effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

  // determine if yield condition is satisfied
  Real yield_condition = effective_trial_stress - _hardening_variable_old[_qp] - _yield_stress;

  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];

  if (yield_condition >
      0) // then use newton iteration to determine effective plastic strain increment
  {
    unsigned int it = 0;
    Real plastic_residual = 0;
    Real norm_plas_residual = 10;
    Real first_norm_plas_residual = 10;
    Real scalar_plastic_strain_increment = 0;

    while (it < _max_its && norm_plas_residual > _absolute_tolerance &&
           (norm_plas_residual / first_norm_plas_residual) > _relative_tolerance)
    {
      plastic_residual = effective_trial_stress -
                         (3. * _shear_modulus * scalar_plastic_strain_increment) -
                         _hardening_variable[_qp] - _yield_stress;
      norm_plas_residual = std::abs(plastic_residual);
      if (it == 0)
      {
        first_norm_plas_residual = norm_plas_residual;
      }

      scalar_plastic_strain_increment +=
          plastic_residual / (3. * _shear_modulus + _hardening_constant);

      _hardening_variable[_qp] =
          _hardening_variable_old[_qp] + (_hardening_constant * scalar_plastic_strain_increment);

      if (_output_iteration_info == true)
      {
        _console << "pls_it=" << it << " trl_strs=" << effective_trial_stress
                 << " del_p=" << scalar_plastic_strain_increment
                 << " harden=" << _hardening_variable[_qp]
                 << " rel_res=" << norm_plas_residual / first_norm_plas_residual
                 << " rel_tol=" << _relative_tolerance << " abs_res=" << norm_plas_residual
                 << " abs_tol=" << _absolute_tolerance << std::endl;
      }

      ++it;
    }

    if (it == _max_its && norm_plas_residual > _absolute_tolerance &&
        (norm_plas_residual / first_norm_plas_residual) > _relative_tolerance)
    {
      mooseError("Max sub-newton iteration hit during plasticity increment solve!");
    }

    if (effective_trial_stress < 0.01)
    {
      effective_trial_stress = 0.01;
    }
    plastic_strain_increment = dev_trial_stress;
    plastic_strain_increment *= (1.5 * scalar_plastic_strain_increment / effective_trial_stress);

    SymmTensor elastic_strain_increment(strain_increment);
    elastic_strain_increment -= plastic_strain_increment;

    // compute stress increment
    stress_new = *elasticityTensor() * elastic_strain_increment;

    // update stress and plastic strain
    stress_new += _stress_old;
    _plastic_strain[_qp] += plastic_strain_increment;

  } // end of if statement
}
