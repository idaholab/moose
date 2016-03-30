/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RecomputeRadialReturnStressIncrement.h"

template<>
InputParameters validParams<RecomputeRadialReturnStressIncrement>()
{
  InputParameters params = validParams<RecomputeGeneralReturnStressIncrement>();

  // Newton Iteration control parameters
  params.addParam<bool>("output_iteration_info", false, "Set true to output newton iteration information from the radial return material");
  params.addParam<bool>("output_iteration_info_on_error", false, "Set true to output the recompute material iteration information when a step fails");
  params.addParam<Real>("relative_tolerance", 1e-8, "Relative convergence tolerance for the newton iteration within the radial return material");
  params.addParam<Real>("absolute_tolerance", 1e-20, "Absolute convergence tolerance for newton iteration within the radial return material");
  return params;
}

RecomputeRadialReturnStressIncrement::RecomputeRadialReturnStressIncrement(const InputParameters & parameters) :
    RecomputeGeneralReturnStressIncrement(parameters),
    _output_iteration_info(getParam<bool>("output_iteration_info")),
    _output_iteration_info_on_error(getParam<bool>("output_iteration_info_on_error")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance"))
{
}

void
RecomputeRadialReturnStressIncrement::computeStressIncrement()
{
  // Given the stretching, compute the stress increment and add iteration to the old stress. Also update the inelastic strain
  // Compute the stress in the intermediate configuration while retaining the stress history
  _return_stress_increment[_qp] = _elasticity_tensor[_qp] * _strain_increment[_qp] + _stress_old[_qp];

  // compute the deviatoric trial stress and trial strain from the current intermediate configuration
  RankTwoTensor deviatoric_trial_stress = _return_stress_increment[_qp].deviatoric();

  // compute the effective trial stress
  Real dev_trial_stress_squared = deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress);
  Real effective_trial_stress = std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);

  //If the effective trial stress is zero, so should the inelastic strain increment be zero
  //In that case skip the entire iteration if the effective trial stress is zero
  if (effective_trial_stress != 0.0)
  {
    computeStressInitialize(effective_trial_stress);

    // Use Newton sub-iteration to determine the scalar effective inelastic strain increment
    Real scalar_effective_inelastic_strain = 0;
    unsigned int iteration = 0;
    Real residual = 10;  // use a large number here to guarantee at least one loop through while
    Real norm_residual = 10;
    Real first_norm_residual = 10;

    // create an output string with iteration information when errors occur
    std::string iteration_output;

    while (iteration < _max_its &&
          norm_residual > _absolute_tolerance &&
          (norm_residual/first_norm_residual) > _relative_tolerance)
    {
      iterationInitialize(scalar_effective_inelastic_strain);

      residual = computeResidual(effective_trial_stress, scalar_effective_inelastic_strain);
      norm_residual = std::abs(residual);
      if (iteration == 0)
      {
        first_norm_residual = norm_residual;
        if (first_norm_residual == 0)
          first_norm_residual = 1;
      }

      Real derivative = computeDerivative(effective_trial_stress, scalar_effective_inelastic_strain);

      scalar_effective_inelastic_strain -= residual / derivative;

      if (_output_iteration_info || _output_iteration_info_on_error)
      {
        iteration_output = "In the element " + Moose::stringify(_current_elem->id()) +
                         + " and the qp point " + Moose::stringify(_qp) + ": \n" +
                         + " iteration = " + Moose::stringify(iteration ) + "\n" +
                         + " effective trial stress = " + Moose::stringify(effective_trial_stress) + "\n" +
                         + " scalar effective inelastic strain = " + Moose::stringify(scalar_effective_inelastic_strain) +"\n" +
                         + " relative residual = " + Moose::stringify(norm_residual/first_norm_residual) + "\n" +
                         + " relative tolerance = " + Moose::stringify(_relative_tolerance) + "\n" +
                         + " absolute residual = " + Moose::stringify(norm_residual) + "\n" +
                         + " absolute tolerance = " + Moose::stringify(_absolute_tolerance) + "\n";
      }

      iterationFinalize(scalar_effective_inelastic_strain);
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

      mooseError("Exceeded maximum iterations in RecomputeRadialReturnStressIncrement solve for material: " << _name << ".  Rerun with  'output_iteration_info_on_error = true' for more information.");
    }

    // compute inelastic strain increments while avoiding a potential divide by zero
      _inelastic_strain_increment[_qp] = deviatoric_trial_stress;
      _inelastic_strain_increment[_qp] *= (3.0 / 2.0 * scalar_effective_inelastic_strain / effective_trial_stress);
  }
  else
    _inelastic_strain_increment[_qp].zero();

  // compute stress update
  _return_stress_increment[_qp] = _elasticity_tensor[_qp] * (_strain_increment[_qp] - _inelastic_strain_increment[_qp]);

  computeStressFinalize(_inelastic_strain_increment[_qp]);
}
