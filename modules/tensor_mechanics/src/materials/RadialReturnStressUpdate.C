//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialReturnStressUpdate.h"

#include "MooseMesh.h"
#include "ElasticityTensorTools.h"

template <>
InputParameters
validParams<RadialReturnStressUpdate>()
{
  InputParameters params = validParams<StressUpdateBase>();
  params.addClassDescription("Calculates the effective inelastic strain increment required to "
                             "return the isotropic stress state to a J2 yield surface.  This class "
                             "is intended to be a parent class for classes with specific "
                             "constitutive models.");
  params += validParams<SingleVariableReturnMappingSolution>();
  params.addParam<Real>("max_inelastic_increment",
                        1e-4,
                        "The maximum inelastic strain increment allowed in a time step");
  return params;
}

RadialReturnStressUpdate::RadialReturnStressUpdate(const InputParameters & parameters,
                                                   const std::string inelastic_strain_name)
  : StressUpdateBase(parameters),
    SingleVariableReturnMappingSolution(parameters),
    _effective_inelastic_strain(
        declareProperty<Real>("effective_" + inelastic_strain_name + "_strain")),
    _effective_inelastic_strain_old(
        getMaterialPropertyOld<Real>("effective_" + inelastic_strain_name + "_strain")),
    _max_inelastic_increment(parameters.get<Real>("max_inelastic_increment"))
{
}

void
RadialReturnStressUpdate::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0;
}

void
RadialReturnStressUpdate::propagateQpStatefulPropertiesRadialReturn()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
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
  // compute the deviatoric trial stress and trial strain from the current intermediate
  // configuration
  RankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  // compute the effective trial stress
  Real dev_trial_stress_squared =
      deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress);
  Real effective_trial_stress = std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);

  // Set the value of 3 * shear modulus for use as a reference residual value
  _three_shear_modulus = 3.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  computeStressInitialize(effective_trial_stress, elasticity_tensor);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  Real scalar_effective_inelastic_strain = 0;
  returnMappingSolve(effective_trial_stress, scalar_effective_inelastic_strain, _console);

  if (scalar_effective_inelastic_strain != 0.0)
    inelastic_strain_increment = deviatoric_trial_stress *
                                 (1.5 * scalar_effective_inelastic_strain / effective_trial_stress);
  else
    inelastic_strain_increment.zero();

  strain_increment -= inelastic_strain_increment;
  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + scalar_effective_inelastic_strain;

  // Use the old elastic strain here because we require tensors used by this class
  // to be isotropic and this method natively allows for changing in time
  // elasticity tensors
  stress_new = elasticity_tensor * (strain_increment + elastic_strain_old);

  computeStressFinalize(inelastic_strain_increment);

  /**
   * Note!  The tangent operator for this class, and derived class is
   * currently just the elasticity tensor, irrespective of compute_full_tangent_operator
   */
  tangent_operator = elasticity_tensor;
}

Real
RadialReturnStressUpdate::computeReferenceResidual(const Real effective_trial_stress,
                                                   const Real scalar_effective_inelastic_strain)
{
  return effective_trial_stress / _three_shear_modulus - scalar_effective_inelastic_strain;
}

Real
RadialReturnStressUpdate::maximumPermissibleValue(const Real effective_trial_stress) const
{
  return effective_trial_stress / _three_shear_modulus;
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
