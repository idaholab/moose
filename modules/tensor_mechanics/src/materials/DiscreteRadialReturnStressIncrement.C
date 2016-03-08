/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DiscreteRadialReturnStressIncrement.h"

template<>
InputParameters validParams<DiscreteRadialReturnStressIncrement>()
{
  InputParameters params = validParams<DiscreteMaterial>();

  // Sub-Newton Iteration control parameters
  params.addParam<unsigned int>("max_iterations", 30, "Maximum number of sub-newton iterations");
  params.addParam<bool>("output_iteration_info", false, "Set true to output sub-newton iteration information");
  params.addParam<bool>("output_iteration_info_on_error", false, "Set true to output the discrete material iteration information when a step fails");
  params.addParam<Real>("relative_tolerance", 1e-5, "Relative convergence tolerance for sub-newtion iteration");
  params.addParam<Real>("absolute_tolerance", 1e-20, "Absolute convergence tolerance for sub-newtion iteration");

  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");

  return params;
}


DiscreteRadialReturnStressIncrement::DiscreteRadialReturnStressIncrement( const InputParameters & parameters)
  :DiscreteMaterial(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _radial_return_stress(declareProperty<RankTwoTensor>("radial_return_stress")),
    _inelastic_strain_increment(declareProperty<RankTwoTensor>("inelastic_strain_increment")),
    _max_its(parameters.get<unsigned int>("max_iterations")),
    _output_iteration_info(getParam<bool>("output_iteration_info")),
    _output_iteration_info_on_error(getParam<bool>("output_iteration_info_on_error")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _elasticity_tensor(getMaterialPropertyByName<ElasticityTensorR4>(_base_name + "elasticity_tensor")),
    _strain_increment(getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _stress(getMaterialPropertyByName<RankTwoTensor>(_base_name + "stress")),
    _stress_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "stress"))
{
}

void
DiscreteRadialReturnStressIncrement::resetQpProperties()
{
  /// Values here to be set to a constant, ideally zero, as in the initQpProperties method of non-discrete materials
  _radial_return_stress[_qp].zero();
  _inelastic_strain_increment[_qp].zero();
}

void
DiscreteRadialReturnStressIncrement::computeQpProperties()
{
  // Given the stretching, compute the stress increment and add iteration to the old stress. Also update the creep strain
  // stress = stressOld + stressIncrement
  if (_t_step == 0)
    return;

  // Compute the stress in the intermediate configuration while retaining the stress history
  _radial_return_stress[_qp] = _elasticity_tensor[_qp] * _strain_increment[_qp] + _stress_old[_qp];

  // compute the deviatoric trial stress and trial strain from the current intermediate configuration
  RankTwoTensor deviatoric_trial_stress = _radial_return_stress[_qp].deviatoric();

  // compute the effective trial stress
  Real dev_trial_stress_squared = deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress);
  Real effective_trial_stress = std::sqrt( 3.0 / 2.0 * dev_trial_stress_squared);

  computeStressInitialize(effective_trial_stress);

  // Use Newton sub-iteration to determine the scalar effective inelastic strain increment
  Real scalar_effective_inelastic_strain = 0;
  unsigned int iteration = 0;
  Real residual = 10;
  Real norm_residual = 10;
  Real first_norm_residual = 10;

  // create an output string with iteration information when errors occur
  std::string iteration_output;

  while (iteration < _max_its &&
        norm_residual > _absolute_tolerance &&
        (norm_residual/first_norm_residual) > _relative_tolerance)
  {
    iterationInitialize( scalar_effective_inelastic_strain );

    residual = computeResidual( effective_trial_stress, scalar_effective_inelastic_strain);
    norm_residual = std::abs(residual);
    if (iteration == 0)
    {
      first_norm_residual = norm_residual;
      if (first_norm_residual == 0)
        first_norm_residual = 1;
    }

    scalar_effective_inelastic_strain -= residual / computeDerivative(effective_trial_stress, scalar_effective_inelastic_strain);

    if (_output_iteration_info == true ||
        _output_iteration_info_on_error == true)
    {
      iteration_output = " iteration = " + Moose::stringify( iteration ) + "\n" +
                       + " effective trial stress = " + Moose::stringify( effective_trial_stress ) + "\n" +
                       + " scalar effective inelastic strain = " + Moose::stringify( scalar_effective_inelastic_strain ) +"\n" +
                       + " relative residual = " + Moose::stringify( norm_residual/first_norm_residual ) + "\n" +
                       + " relative tolerance = " + Moose::stringify( _relative_tolerance ) + "\n" +
                       + " absolute residual = " + Moose::stringify( norm_residual ) + "\n" +
                       + " absolute tolerance = " + Moose::stringify( _absolute_tolerance );
    }

    iterationFinalize( scalar_effective_inelastic_strain );
    ++iteration;
  }

  if (_output_iteration_info)
    _console << iteration_output << std::endl;

  if (iteration == _max_its &&
     norm_residual > _absolute_tolerance &&
     (norm_residual/first_norm_residual) > _relative_tolerance)
  {
    if (_output_iteration_info_on_error)
      Moose::err << iteration_output;

    mooseError("During the material iteration in the DiscreteRadialReturn material, the max number of iterations was hit during nonlinear constitutive model solve on (" << _name << ").  Try running this problem with the option 'output_iteration_info_on_error' set to true in the input file to output more information.");
  }

  // compute inelastic strain increments (avoid potential divide by zero - how should this be done)?
  if (effective_trial_stress < 0.001)
    effective_trial_stress = 0.001;

  _inelastic_strain_increment[_qp] = deviatoric_trial_stress;
  _inelastic_strain_increment[_qp] *= (3.0 / 2.0 * scalar_effective_inelastic_strain / effective_trial_stress);

  // compute stress update
  _radial_return_stress[_qp] = _elasticity_tensor[_qp] * (_strain_increment[_qp] - _inelastic_strain_increment[_qp]);

  computeStressFinalize(_inelastic_strain_increment[_qp]);
}
