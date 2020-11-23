//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRadialReturnStressUpdate.h"

#include "MooseMesh.h"
#include "ElasticityTensorTools.h"

InputParameters
ADRadialReturnStressUpdate::validParams()
{
  InputParameters params = ADStressUpdateBase::validParams();
  params.addClassDescription("Calculates the effective inelastic strain increment required to "
                             "return the isotropic stress state to a J2 yield surface.  This class "
                             "is intended to be a parent class for classes with specific "
                             "constitutive models.");
  params += ADSingleVariableReturnMappingSolution::validParams();
  params.addParam<Real>("max_inelastic_increment",
                        1e-4,
                        "The maximum inelastic strain increment allowed in a time step");
  params.addRequiredParam<std::string>(
      "effective_inelastic_strain_name",
      "Name of the material property that stores the effective inelastic strain");
  params.addParam<bool>("use_substep", false, "Whether to use substepping");
  params.addParam<Real>("substep_strain_tolerance",
                        0.1,
                        "Maximum ratio of the initial elastic strain increment at start of the "
                        "return mapping solve to the maximum inelastic strain allowable in a "
                        "single substep. Reduce this value to increase the number of substeps");
  params.addParam<bool>("apply_strain", true, "Flag to apply strain. Used for testing.");
  params.addParamNamesToGroup(
      "effective_inelastic_strain_name substep_strain_tolerance apply_strain", "Advanced");
  return params;
}

ADRadialReturnStressUpdate::ADRadialReturnStressUpdate(const InputParameters & parameters)
  : ADStressUpdateBase(parameters),
    ADSingleVariableReturnMappingSolution(parameters),
    _effective_inelastic_strain(declareADProperty<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _effective_inelastic_strain_old(getMaterialPropertyOld<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _max_inelastic_increment(getParam<Real>("max_inelastic_increment")),
    _substep_tolerance(getParam<Real>("substep_strain_tolerance")),
    _apply_strain(getParam<bool>("apply_strain"))
{
}

void
ADRadialReturnStressUpdate::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
}

void
ADRadialReturnStressUpdate::propagateQpStatefulPropertiesRadialReturn()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
}

int
ADRadialReturnStressUpdate::calculateNumberSubsteps(const ADRankTwoTensor & strain_increment)
{
  unsigned int substep_number = 1;
  // compute an effective elastic strain measure
  const ADReal contracted_elastic_strain = strain_increment.doubleContraction(strain_increment);
  const Real effective_elastic_strain =
      std::sqrt(3.0 / 2.0 * MetaPhysicL::raw_value(contracted_elastic_strain));

  if (!MooseUtils::absoluteFuzzyEqual(effective_elastic_strain, 0.0))
  {
    const Real ratio = effective_elastic_strain / _max_inelastic_increment;

    if (ratio > _substep_tolerance)
      substep_number = std::ceil(ratio / _substep_tolerance);
  }

  return substep_number;
}

void
ADRadialReturnStressUpdate::updateState(ADRankTwoTensor & strain_increment,
                                        ADRankTwoTensor & inelastic_strain_increment,
                                        const ADRankTwoTensor & /*rotation_increment*/,
                                        ADRankTwoTensor & stress_new,
                                        const RankTwoTensor & /*stress_old*/,
                                        const ADRankFourTensor & elasticity_tensor,
                                        const RankTwoTensor & elastic_strain_old)
{
  // compute the deviatoric trial stress and trial strain from the current intermediate
  // configuration
  ADRankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  // compute the effective trial stress
  ADReal dev_trial_stress_squared =
      deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress);
  ADReal effective_trial_stress =
      dev_trial_stress_squared == 0.0 ? 0.0 : std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);

  // Set the value of 3 * shear modulus for use as a reference residual value
  _three_shear_modulus = 3.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  computeStressInitialize(effective_trial_stress, elasticity_tensor);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  _scalar_effective_inelastic_strain = 0.0;
  if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
  {
    returnMappingSolve(effective_trial_stress, _scalar_effective_inelastic_strain, _console);
    if (_scalar_effective_inelastic_strain != 0.0)
      inelastic_strain_increment =
          deviatoric_trial_stress *
          (1.5 * _scalar_effective_inelastic_strain / effective_trial_stress);
    else
      inelastic_strain_increment.zero();
  }
  else
    inelastic_strain_increment.zero();

  if (_apply_strain)
  {
    strain_increment -= inelastic_strain_increment;
    _effective_inelastic_strain[_qp] =
        _effective_inelastic_strain_old[_qp] + _scalar_effective_inelastic_strain;

    // Use the old elastic strain here because we require tensors used by this class
    // to be isotropic and this method natively allows for changing in time
    // elasticity tensors
    stress_new = elasticity_tensor * (elastic_strain_old + strain_increment);
  }

  computeStressFinalize(inelastic_strain_increment);
}

void
ADRadialReturnStressUpdate::updateStateSubstep(ADRankTwoTensor & strain_increment,
                                               ADRankTwoTensor & inelastic_strain_increment,
                                               const ADRankTwoTensor & rotation_increment,
                                               ADRankTwoTensor & stress_new,
                                               const RankTwoTensor & stress_old,
                                               const ADRankFourTensor & elasticity_tensor,
                                               const RankTwoTensor & elastic_strain_old)
{
  const unsigned int total_substeps = calculateNumberSubsteps(strain_increment);

  // if only one substep is needed, then call the original update state method
  if (total_substeps == 1)
  {
    updateState(strain_increment,
                inelastic_strain_increment,
                rotation_increment,
                stress_new,
                stress_old,
                elasticity_tensor,
                elastic_strain_old);
    return;
  }
  // Store original _dt; Reset at the end of solve
  Real dt_original = _dt;
  // cut the original timestep
  _dt = dt_original / total_substeps;

  // initialize the inputs
  const ADRankTwoTensor strain_increment_per_step = strain_increment / total_substeps;
  ADRankTwoTensor sub_stress_new = elasticity_tensor * elastic_strain_old;

  ADRankTwoTensor sub_elastic_strain_old = elastic_strain_old;
  ADRankTwoTensor sub_inelastic_strain_increment = inelastic_strain_increment;

  ADReal sub_scalar_effective_inelastic_strain = 0;

  // clear the original inputs
  MathUtils::mooseSetToZero(strain_increment);
  MathUtils::mooseSetToZero(inelastic_strain_increment);
  MathUtils::mooseSetToZero(stress_new);

  for (unsigned int step = 0; step < total_substeps; ++step)
  {
    // set up input for this substep
    ADRankTwoTensor sub_strain_increment = strain_increment_per_step;
    sub_stress_new += elasticity_tensor * sub_strain_increment;

    // update stress and strain based on the strain increment
    updateState(sub_strain_increment,
                sub_inelastic_strain_increment,
                rotation_increment, // not used in updateState
                sub_stress_new,
                stress_old, // not used in updateState
                elasticity_tensor,
                elastic_strain_old);
    // update strain and stress
    strain_increment += sub_strain_increment;
    inelastic_strain_increment += sub_inelastic_strain_increment;
    sub_elastic_strain_old += sub_strain_increment;
    sub_stress_new = elasticity_tensor * sub_elastic_strain_old;
    // accumulate scalar_effective_inelastic_strain
    sub_scalar_effective_inelastic_strain += _scalar_effective_inelastic_strain;
    computeStressFinalize(inelastic_strain_increment);
    // store incremental material properties for this step
    storeIncrementalMaterialProperties();
  }
  // update stress
  stress_new = sub_stress_new;
  // update effective inelastic strain
  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + sub_scalar_effective_inelastic_strain;

  // recover the original timestep
  _dt = dt_original;
}

Real
ADRadialReturnStressUpdate::computeReferenceResidual(
    const ADReal & effective_trial_stress, const ADReal & scalar_effective_inelastic_strain)
{
  return MetaPhysicL::raw_value(effective_trial_stress / _three_shear_modulus) -
         MetaPhysicL::raw_value(scalar_effective_inelastic_strain);
}

ADReal
ADRadialReturnStressUpdate::maximumPermissibleValue(const ADReal & effective_trial_stress) const
{
  return effective_trial_stress / _three_shear_modulus;
}

Real
ADRadialReturnStressUpdate::computeTimeStepLimit()
{
  const Real scalar_inelastic_strain_incr =
      std::abs(MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
               _effective_inelastic_strain_old[_qp]);
  if (!scalar_inelastic_strain_incr)
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}

void
ADRadialReturnStressUpdate::outputIterationSummary(std::stringstream * iter_output,
                                                   const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  }
  ADSingleVariableReturnMappingSolution::outputIterationSummary(iter_output, total_it);
}
