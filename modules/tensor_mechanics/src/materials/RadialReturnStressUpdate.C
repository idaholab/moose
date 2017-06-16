/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "RadialReturnStressUpdate.h"

#include "MooseMesh.h"

template <>
InputParameters
validParams<RadialReturnStressUpdate>()
{
  InputParameters params = validParams<StressUpdateBase>();
  params.addClassDescription("Calculates the effective inelastic strain increment required to "
                             "return the isotropic stress state to a J2 yield surface.  This class "
                             "is intended to be a parent class for classes with specific "
                             "constitutive models.");
  // Newton Iteration control parameters
  params.addParam<bool>(
      "output_iteration_info",
      false,
      "Set true to output newton iteration information from the radial return material");
  params.addParam<bool>(
      "output_iteration_info_on_error",
      false,
      "Set true to output the recompute material iteration information when a step fails");
  params.addParam<Real>(
      "relative_tolerance",
      1e-8,
      "Relative convergence tolerance for the newton iteration within the radial return material");
  params.addParam<Real>(
      "absolute_tolerance",
      1e-20,
      "Absolute convergence tolerance for newton iteration within the radial return material");
  params.addParam<unsigned int>(
      "max_iterations", 30, "Maximum number of newton iterations in the radial return material");
  params.addParam<Real>("max_inelastic_increment",
                        1e-4,
                        "The maximum inelastic strain increment allowed in a time step");
  return params;
}

RadialReturnStressUpdate::RadialReturnStressUpdate(const InputParameters & parameters,
                                                   const std::string inelastic_strain_name)
  : StressUpdateBase(parameters),
    _max_its(parameters.get<unsigned int>("max_iterations")),
    _output_iteration_info(getParam<bool>("output_iteration_info")),
    _output_iteration_info_on_error(getParam<bool>("output_iteration_info_on_error")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _effective_inelastic_strain(
        declareProperty<Real>("effective_" + inelastic_strain_name + "_strain")),
    _effective_inelastic_strain_old(
        declarePropertyOld<Real>("effective_" + inelastic_strain_name + "_strain")),
    _max_inelastic_increment(parameters.get<Real>("max_inelastic_increment"))
{
}

void
RadialReturnStressUpdate::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0;
}

void
RadialReturnStressUpdate::updateState(RankTwoTensor & strain_increment,
                                      RankTwoTensor & inelastic_strain_increment,
                                      const RankTwoTensor & /*rotation_increment*/,
                                      RankTwoTensor & stress_new,
                                      const RankTwoTensor & /*stress_old*/,
                                      const RankFourTensor & elasticity_tensor,
                                      const RankTwoTensor & elastic_strain_old,
                                      bool /*compute_full_tangent_operator*/,
                                      RankFourTensor & tangent_operator)
{
  // Given the stretching, update the inelastic strain
  // Compute the stress in the intermediate configuration while retaining the stress history

  // compute the deviatoric trial stress and trial strain from the current intermediate
  // configuration
  RankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  // compute the effective trial stress
  Real dev_trial_stress_squared =
      deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress);
  Real effective_trial_stress = std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);

  Real scalar_effective_inelastic_strain = 0;

  // If the effective trial stress is zero, so should the inelastic strain increment be zero
  // In that case skip the entire iteration if the effective trial stress is zero
  if (effective_trial_stress != 0.0)
  {
    computeStressInitialize(effective_trial_stress, elasticity_tensor);

    // Use Newton iteration to determine the scalar effective inelastic strain increment
    unsigned int iteration = 0;

    Real residual, norm_residual, first_norm_residual = 0;

    // create an output string with iteration information when errors occur
    std::string iteration_output;

    do
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

      Real derivative =
          computeDerivative(effective_trial_stress, scalar_effective_inelastic_strain);

      scalar_effective_inelastic_strain -= residual / derivative;

      if (_output_iteration_info || _output_iteration_info_on_error)
      {
        iteration_output =
            "In the element " + Moose::stringify(_current_elem->id()) + +" and the qp point " +
            Moose::stringify(_qp) + ": \n" + +" iteration = " + Moose::stringify(iteration) + "\n" +
            +" effective trial stress = " + Moose::stringify(effective_trial_stress) + "\n" +
            +" scalar effective inelastic strain = " +
            Moose::stringify(scalar_effective_inelastic_strain) + "\n" + +" relative residual = " +
            Moose::stringify(norm_residual / first_norm_residual) + "\n" +
            +" relative tolerance = " + Moose::stringify(_relative_tolerance) + "\n" +
            +" absolute residual = " + Moose::stringify(norm_residual) + "\n" +
            +" absolute tolerance = " + Moose::stringify(_absolute_tolerance) + "\n";
      }

      iterationFinalize(scalar_effective_inelastic_strain);
      ++iteration;
    } while (iteration < _max_its && norm_residual > _absolute_tolerance &&
             (norm_residual / first_norm_residual) > _relative_tolerance);

    if (_output_iteration_info)
      _console << iteration_output << std::endl;

    if (iteration == _max_its && norm_residual > _absolute_tolerance &&
        (norm_residual / first_norm_residual) > _relative_tolerance)
    {
      if (_output_iteration_info_on_error)
        Moose::err << iteration_output;

      mooseError("Exceeded maximum iterations in RadialReturnStressUpdate solve for material: ",
                 _name,
                 ".  Rerun with  'output_iteration_info_on_error = true' for more information.");
    }

    // compute inelastic strain increments while avoiding a potential divide by zero
    inelastic_strain_increment = deviatoric_trial_stress;
    inelastic_strain_increment *=
        (3.0 / 2.0 * scalar_effective_inelastic_strain / effective_trial_stress);
  }
  else
    inelastic_strain_increment.zero();

  strain_increment -= inelastic_strain_increment;
  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + scalar_effective_inelastic_strain;
  stress_new = elasticity_tensor * (strain_increment + elastic_strain_old);

  computeStressFinalize(inelastic_strain_increment);

  /**
   * Note!  The tangent operator for this class, and derived class is
   * currently just the elasticity tensor, irrespective of compute_full_tangent_operator
   */
  tangent_operator = elasticity_tensor;
}

Real
RadialReturnStressUpdate::computeTimeStepLimit()
{
  Real scalar_inelastic_strain_incr;

  scalar_inelastic_strain_incr =
      _effective_inelastic_strain[_qp] - _effective_inelastic_strain_old[_qp];
  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}
