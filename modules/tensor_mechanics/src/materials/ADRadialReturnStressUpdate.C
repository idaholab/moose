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

defineADValidParams(
    ADRadialReturnStressUpdate,
    ADStressUpdateBase,
    params.addClassDescription(
        "Calculates the effective inelastic strain increment required to "
        "return the isotropic stress state to a J2 yield surface.  This class "
        "is intended to be a parent class for classes with specific "
        "constitutive models.");
    params += validParams<ADSingleVariableReturnMappingSolution<RESIDUAL>>();
    params.addParam<Real>("max_inelastic_increment",
                          1e-4,
                          "The maximum inelastic strain increment allowed in a time step");
    params.addRequiredParam<std::string>(
        "effective_inelastic_strain_name",
        "Name of the material property that stores the effective inelastic strain");
    params.addParamNamesToGroup("effective_inelastic_strain_name", "Advanced"););

template <ComputeStage compute_stage>
ADRadialReturnStressUpdate<compute_stage>::ADRadialReturnStressUpdate(
    const InputParameters & parameters)
  : ADStressUpdateBase<compute_stage>(parameters),
    ADSingleVariableReturnMappingSolution<compute_stage>(parameters),
    _effective_inelastic_strain(adDeclareADProperty<Real>(
        _base_name + adGetParam<std::string>("effective_inelastic_strain_name"))),
    _effective_inelastic_strain_old(adGetMaterialPropertyOld<Real>(
        _base_name + adGetParam<std::string>("effective_inelastic_strain_name"))),
    _max_inelastic_increment(adGetParam<Real>("max_inelastic_increment"))
{
}

template <ComputeStage compute_stage>
void
ADRadialReturnStressUpdate<compute_stage>::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
}

template <ComputeStage compute_stage>
void
ADRadialReturnStressUpdate<compute_stage>::propagateQpStatefulPropertiesRadialReturn()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
}

template <ComputeStage compute_stage>
void
ADRadialReturnStressUpdate<compute_stage>::updateState(
    ADRankTwoTensor & strain_increment,
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
  ADReal effective_trial_stress = MooseUtils::absoluteFuzzyEqual(dev_trial_stress_squared, 0.0)
                                      ? 0.0
                                      : std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);

  // Set the value of 3 * shear modulus for use as a reference residual value
  _three_shear_modulus = 3.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  computeStressInitialize(effective_trial_stress, elasticity_tensor);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  ADReal scalar_effective_inelastic_strain = 0.0;
  if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
  {
    returnMappingSolve(effective_trial_stress, scalar_effective_inelastic_strain, _console);
    if (scalar_effective_inelastic_strain != 0.0)
      inelastic_strain_increment =
          deviatoric_trial_stress *
          (1.5 * scalar_effective_inelastic_strain / effective_trial_stress);
    else
      inelastic_strain_increment.zero();
  }
  else
    inelastic_strain_increment.zero();

  strain_increment -= inelastic_strain_increment;
  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + scalar_effective_inelastic_strain;

  // Use the old elastic strain here because we require tensors used by this class
  // to be isotropic and this method natively allows for changing in time
  // elasticity tensors
  stress_new = elasticity_tensor * (elastic_strain_old + strain_increment);

  computeStressFinalize(inelastic_strain_increment);
}

template <ComputeStage compute_stage>
Real
ADRadialReturnStressUpdate<compute_stage>::computeReferenceResidual(
    const ADReal & effective_trial_stress, const ADReal & scalar_effective_inelastic_strain)
{
  return MetaPhysicL::raw_value(effective_trial_stress / _three_shear_modulus) -
         MetaPhysicL::raw_value(scalar_effective_inelastic_strain);
}

template <ComputeStage compute_stage>
ADReal
ADRadialReturnStressUpdate<compute_stage>::maximumPermissibleValue(
    const ADReal & effective_trial_stress) const
{
  return effective_trial_stress / _three_shear_modulus;
}

template <ComputeStage compute_stage>
Real
ADRadialReturnStressUpdate<compute_stage>::computeTimeStepLimit()
{
  Real scalar_inelastic_strain_incr = MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
                                      _effective_inelastic_strain_old[_qp];
  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}

template <ComputeStage compute_stage>
void
ADRadialReturnStressUpdate<compute_stage>::outputIterationSummary(std::stringstream * iter_output,
                                                                  const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  }
  ADSingleVariableReturnMappingSolution<compute_stage>::outputIterationSummary(iter_output,
                                                                               total_it);
}

// explicit instantiation is required for AD base classes
adBaseClass(ADRadialReturnStressUpdate);
