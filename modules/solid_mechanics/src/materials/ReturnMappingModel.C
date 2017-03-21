/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ReturnMappingModel.h"

#include "SymmIsotropicElasticityTensor.h"
#include "Conversion.h"

template <>
InputParameters
validParams<ReturnMappingModel>()
{
  InputParameters params = validParams<ConstitutiveModel>();

  // Sub-Newton Iteration control parameters
  params.addParam<unsigned int>("max_its", 30, "Maximum number of sub-newton iterations");
  params.addParam<bool>(
      "output_iteration_info", false, "Set true to output sub-newton iteration information");
  params.addParam<bool>("output_iteration_info_on_error",
                        false,
                        "Set true to output sub-newton iteration information when a step fails");
  params.addParam<Real>(
      "relative_tolerance", 1e-5, "Relative convergence tolerance for sub-newtion iteration");
  params.addParam<Real>(
      "absolute_tolerance", 1e-20, "Absolute convergence tolerance for sub-newtion iteration");

  return params;
}

ReturnMappingModel::ReturnMappingModel(const InputParameters & parameters)
  : ConstitutiveModel(parameters),
    _max_its(parameters.get<unsigned int>("max_its")),
    _output_iteration_info(getParam<bool>("output_iteration_info")),
    _output_iteration_info_on_error(getParam<bool>("output_iteration_info_on_error")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _effective_strain_increment(0)
{
}

void
ReturnMappingModel::computeStress(const Elem & current_elem,
                                  unsigned qp,
                                  const SymmElasticityTensor & elasticityTensor,
                                  const SymmTensor & stress_old,
                                  SymmTensor & strain_increment,
                                  SymmTensor & stress_new)
{
  // Given the stretching, compute the stress increment and add it to the old stress. Also update
  // the creep strain
  // stress = stressOld + stressIncrement
  if (_t_step == 0 && !_app.isRestarting())
    return;

  stress_new = elasticityTensor * strain_increment;
  stress_new += stress_old;

  SymmTensor inelastic_strain_increment;
  computeStress(current_elem,
                qp,
                elasticityTensor,
                stress_old,
                strain_increment,
                stress_new,
                inelastic_strain_increment);
}

void
ReturnMappingModel::computeStress(const Elem & /*current_elem*/,
                                  unsigned qp,
                                  const SymmElasticityTensor & elasticityTensor,
                                  const SymmTensor & stress_old,
                                  SymmTensor & strain_increment,
                                  SymmTensor & stress_new,
                                  SymmTensor & inelastic_strain_increment)
{
  // compute deviatoric trial stress
  SymmTensor dev_trial_stress(stress_new);
  dev_trial_stress.addDiag(-dev_trial_stress.trace() / 3.0);

  // compute effective trial stress
  Real dts_squared = dev_trial_stress.doubleContraction(dev_trial_stress);
  Real effective_trial_stress = std::sqrt(1.5 * dts_squared);

  // compute effective strain increment
  SymmTensor dev_strain_increment(strain_increment);
  dev_strain_increment.addDiag(-strain_increment.trace() / 3.0);
  _effective_strain_increment = dev_strain_increment.doubleContraction(dev_strain_increment);
  _effective_strain_increment = std::sqrt(2.0 / 3.0 * _effective_strain_increment);

  computeStressInitialize(qp, effective_trial_stress, elasticityTensor);

  // Use Newton sub-iteration to determine inelastic strain increment

  Real scalar = 0;
  unsigned int it = 0;
  Real residual = 10;
  Real norm_residual = 10;
  Real first_norm_residual = 10;

  std::string iter_output;

  while (it < _max_its && norm_residual > _absolute_tolerance &&
         (norm_residual / first_norm_residual) > _relative_tolerance)
  {
    iterationInitialize(qp, scalar);

    residual = computeResidual(qp, effective_trial_stress, scalar);
    norm_residual = std::abs(residual);
    if (it == 0)
    {
      first_norm_residual = norm_residual;
      if (first_norm_residual == 0)
      {
        first_norm_residual = 1;
      }
    }

    scalar -= residual / computeDerivative(qp, effective_trial_stress, scalar);

    if (_output_iteration_info == true || _output_iteration_info_on_error == true)
    {
      iter_output =
          "In the element " + Moose::stringify(_current_elem->id()) + +" and the qp point " +
          Moose::stringify(qp) + ": \n" + +" iteration = " + Moose::stringify(it) + "\n" +
          +" effective trial stress = " + Moose::stringify(effective_trial_stress) + "\n" +
          +" scalar effective inelastic strain = " + Moose::stringify(scalar) + "\n" +
          +" relative residual = " + Moose::stringify(norm_residual / first_norm_residual) + "\n" +
          +" relative tolerance = " + Moose::stringify(_relative_tolerance) + "\n" +
          +" absolute residual = " + Moose::stringify(norm_residual) + "\n" +
          +" absolute tolerance = " + Moose::stringify(_absolute_tolerance) + "\n";
    }
    iterationFinalize(qp, scalar);
    ++it;
  }

  if (_output_iteration_info)
    _console << iter_output;

  if (it == _max_its && norm_residual > _absolute_tolerance &&
      (norm_residual / first_norm_residual) > _relative_tolerance)
  {
    if (_output_iteration_info_on_error)
    {
      Moose::err << iter_output;
    }
    mooseError("Exceeded maximum iterations in ReturnMappingModel solve for material: ",
               _name,
               ".  Rerun with  'output_iteration_info_on_error = true' for more information.");
  }

  // compute inelastic and elastic strain increments (avoid potential divide by zero - how should
  // this be done)?
  if (effective_trial_stress < 0.01)
  {
    effective_trial_stress = 0.01;
  }

  inelastic_strain_increment = dev_trial_stress;
  inelastic_strain_increment *= (1.5 * scalar / effective_trial_stress);

  strain_increment -= inelastic_strain_increment;

  // compute stress increment
  stress_new = elasticityTensor * strain_increment;

  // update stress
  stress_new += stress_old;

  computeStressFinalize(qp, inelastic_strain_increment);
}
