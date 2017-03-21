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
  return params;
}

RadialReturnStressUpdate::RadialReturnStressUpdate(const InputParameters & parameters)
  : StressUpdateBase(parameters),
    _max_its(parameters.get<unsigned int>("max_iterations")),
    _output_iteration_info(getParam<bool>("output_iteration_info")),
    _output_iteration_info_on_error(getParam<bool>("output_iteration_info_on_error")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance"))
{
}

void
RadialReturnStressUpdate::updateStress(RankTwoTensor & strain_increment,
                                       RankTwoTensor & inelastic_strain_increment,
                                       RankTwoTensor & stress_new)
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

  // If the effective trial stress is zero, so should the inelastic strain increment be zero
  // In that case skip the entire iteration if the effective trial stress is zero
  if (effective_trial_stress != 0.0)
  {
    computeStressInitialize(effective_trial_stress);

    // Use Newton iteration to determine the scalar effective inelastic strain increment
    Real scalar_effective_inelastic_strain = 0;
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
  stress_new = _elasticity_tensor[_qp] * (strain_increment + _elastic_strain_old[_qp]);

  computeStressFinalize(inelastic_strain_increment);
}

Real
RadialReturnStressUpdate::getIsotropicShearModulus()
{
  const Real shear_modulus = _elasticity_tensor[_qp](0, 1, 0, 1);
  if (_mesh.dimension() == 3 && shear_modulus != _elasticity_tensor[_qp](0, 2, 0, 2))
    mooseError("Check to ensure that your Elasticity Tensor is truly Isotropic");
  return shear_modulus;
}

Real
RadialReturnStressUpdate::getIsotropicBulkModulus()
{
  const Real shear_modulus = getIsotropicShearModulus();
  // dilatational modulus is defined as lambda plus two mu
  const Real dilatational_modulus = _elasticity_tensor[_qp](0, 0, 0, 0);
  if (_mesh.dimension() == 3 && dilatational_modulus != _elasticity_tensor[_qp](2, 2, 2, 2))
    mooseError("Check to ensure that your Elasticity Tensor is truly Isotropic");
  const Real lambda = dilatational_modulus - 2.0 * shear_modulus;
  const Real bulk_modulus = lambda + 2.0 * shear_modulus / 3.0;
  return bulk_modulus;
}
