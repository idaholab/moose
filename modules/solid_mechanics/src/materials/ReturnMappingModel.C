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
  params += validParams<SingleVariableReturnMappingSolution>();
  params.addParam<Real>("max_inelastic_increment",
                        1e-4,
                        "The maximum inelastic strain increment allowed in a time step");
  return params;
}

ReturnMappingModel::ReturnMappingModel(const InputParameters & parameters,
                                       const std::string inelastic_strain_name)
  : ConstitutiveModel(parameters),
    SingleVariableReturnMappingSolution(parameters),
    _effective_strain_increment(0),
    _effective_inelastic_strain(
        declareProperty<Real>("effective_" + inelastic_strain_name + "_strain")),
    _effective_inelastic_strain_old(
        getMaterialPropertyOld<Real>("effective_" + inelastic_strain_name + "_strain")),
    _max_inelastic_increment(parameters.get<Real>("max_inelastic_increment"))
{
}

void
ReturnMappingModel::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
}

void
ReturnMappingModel::computeStress(const Elem & current_elem,
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
                elasticityTensor,
                stress_old,
                strain_increment,
                stress_new,
                inelastic_strain_increment);
}

void
ReturnMappingModel::computeStress(const Elem & /*current_elem*/,
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

  const SymmIsotropicElasticityTensor * iso_e_t =
      dynamic_cast<const SymmIsotropicElasticityTensor *>(&elasticityTensor);
  if (!iso_e_t)
    mooseError("Models derived from ReturnMappingModel require a SymmIsotropicElasticityTensor");
  _three_shear_modulus = 3.0 * iso_e_t->shearModulus();

  computeStressInitialize(effective_trial_stress, elasticityTensor);

  Real scalar;
  returnMappingSolve(effective_trial_stress, scalar, _console);

  // compute inelastic and elastic strain increments
  if (_legacy_return_mapping)
  {
    if (effective_trial_stress < 0.01)
      effective_trial_stress = 0.01;

    inelastic_strain_increment = dev_trial_stress;
    inelastic_strain_increment *= (1.5 * scalar / effective_trial_stress);
  }
  else
  {
    if (scalar != 0.0)
      inelastic_strain_increment = dev_trial_stress * (1.5 * scalar / effective_trial_stress);
    else
      inelastic_strain_increment = 0.0;
  }

  strain_increment -= inelastic_strain_increment;
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp] + scalar;

  // compute stress increment
  stress_new = elasticityTensor * strain_increment;

  // update stress
  stress_new += stress_old;

  computeStressFinalize(inelastic_strain_increment);
}

Real
ReturnMappingModel::computeReferenceResidual(const Real effective_trial_stress, const Real scalar)
{
  return effective_trial_stress / _three_shear_modulus - scalar;
}

Real
ReturnMappingModel::computeTimeStepLimit()
{
  Real scalar_inelastic_strain_incr;

  scalar_inelastic_strain_incr =
      _effective_inelastic_strain[_qp] - _effective_inelastic_strain_old[_qp];
  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}
