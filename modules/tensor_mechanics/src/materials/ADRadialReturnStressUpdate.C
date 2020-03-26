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
  params.addParam<bool>("apply_strain", true, "Flag to apply strain. Used for testing.");
  params.addParamNamesToGroup("effective_inelastic_strain_name apply_strain", "Advanced");
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

  if (_apply_strain)
  {
    strain_increment -= inelastic_strain_increment;
    _effective_inelastic_strain[_qp] =
        _effective_inelastic_strain_old[_qp] + scalar_effective_inelastic_strain;

    // Use the old elastic strain here because we require tensors used by this class
    // to be isotropic and this method natively allows for changing in time
    // elasticity tensors
    stress_new = elasticity_tensor * (elastic_strain_old + strain_increment);
  }

  computeStressFinalize(inelastic_strain_increment);
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
  Real scalar_inelastic_strain_incr = MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
                                      _effective_inelastic_strain_old[_qp];
  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
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
