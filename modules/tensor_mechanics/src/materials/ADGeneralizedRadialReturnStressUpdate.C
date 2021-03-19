//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADGeneralizedRadialReturnStressUpdate.h"

#include "MooseMesh.h"
#include "MooseTypes.h"
#include "ElasticityTensorTools.h"
#include "libmesh/ignore_warnings.h"
#include "Eigen/Dense"
#include "Eigen/Eigenvalues"
#include "libmesh/restore_warnings.h"

InputParameters
ADGeneralizedRadialReturnStressUpdate::validParams()
{
  InputParameters params = ADStressUpdateBase::validParams();
  params.addClassDescription("Calculates the effective inelastic strain increment required to "
                             "return the isotropic stress state to a J2 yield surface.  This class "
                             "is intended to be a parent class for classes with specific "
                             "constitutive models.");
  params += ADGeneralizedReturnMappingSolution::validParams();
  params.addParam<Real>("max_inelastic_increment",
                        1e-4,
                        "The maximum inelastic strain increment allowed in a time step");
  params.addParam<Real>("max_integration_error",
                        5e-4,
                        "The maximum inelastic strain increment integration error allowed");
  params.addRequiredParam<std::string>(
      "effective_inelastic_strain_name",
      "Name of the material property that stores the effective inelastic strain");
  params.addRequiredParam<std::string>(
      "inelastic_strain_rate_name",
      "Name of the material property that stores the inelastic strain rate");
  return params;
}

ADGeneralizedRadialReturnStressUpdate::ADGeneralizedRadialReturnStressUpdate(
    const InputParameters & parameters)
  : ADStressUpdateBase(parameters),
    ADGeneralizedReturnMappingSolution(parameters),
    _effective_inelastic_strain(declareADProperty<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _effective_inelastic_strain_old(getMaterialPropertyOld<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _inelastic_strain_rate(
        declareProperty<Real>(_base_name + getParam<std::string>("inelastic_strain_rate_name"))),
    _inelastic_strain_rate_old(getMaterialPropertyOld<Real>(
        _base_name + getParam<std::string>("inelastic_strain_rate_name"))),
    _max_inelastic_increment(getParam<Real>("max_inelastic_increment")),
    _max_integration_error(getParam<Real>("max_integration_error")),
    _max_integration_error_time_step(std::numeric_limits<Real>::max())
{
}

void
ADGeneralizedRadialReturnStressUpdate::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
  _inelastic_strain_rate[_qp] = 0.0;
}

void
ADGeneralizedRadialReturnStressUpdate::propagateQpStatefulPropertiesRadialReturn()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _inelastic_strain_rate[_qp] = _inelastic_strain_rate_old[_qp];
}

void
ADGeneralizedRadialReturnStressUpdate::updateState(
    ADRankTwoTensor & elastic_strain_increment,
    ADRankTwoTensor & inelastic_strain_increment,
    const ADRankTwoTensor & /*rotation_increment*/,
    ADRankTwoTensor & stress_new,
    const RankTwoTensor & stress_old,
    const ADRankFourTensor & elasticity_tensor,
    const RankTwoTensor & /*elastic_strain_old*/,
    bool /*compute_full_tangent_operator = false*/,
    RankFourTensor & /*tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor*/)
{
  // Prepare initial trial stress for generalized return mapping
  ADRankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  ADDenseVector stress_new_vector(6);
  stress_new_vector(0) = stress_new(0, 0);
  stress_new_vector(1) = stress_new(1, 1);
  stress_new_vector(2) = stress_new(2, 2);
  stress_new_vector(3) = stress_new(0, 1);
  stress_new_vector(4) = stress_new(1, 2);
  stress_new_vector(5) = stress_new(0, 2);

  ADDenseVector stress_dev(6);
  stress_dev(0) = deviatoric_trial_stress(0, 0);
  stress_dev(1) = deviatoric_trial_stress(1, 1);
  stress_dev(2) = deviatoric_trial_stress(2, 2);
  stress_dev(3) = deviatoric_trial_stress(0, 1);
  stress_dev(4) = deviatoric_trial_stress(1, 2);
  stress_dev(5) = deviatoric_trial_stress(0, 2);

  computeStressInitialize(stress_dev, stress_new_vector, elasticity_tensor);

  // Use Newton iteration to determine a plastic multiplier variable
  ADReal delta_gamma = 0.0;

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  if (!MooseUtils::absoluteFuzzyEqual(MetaPhysicL::raw_value(stress_dev).l2_norm(), 0.0))
  {
    returnMappingSolve(stress_dev, stress_new_vector, delta_gamma, _console);

    if (delta_gamma != 0.0)
      computeStrainFinalize(inelastic_strain_increment, stress_new, stress_dev, delta_gamma);
    else
      inelastic_strain_increment.zero();
  }
  else
    inelastic_strain_increment.zero();

  elastic_strain_increment -= inelastic_strain_increment;

  computeStressFinalize(inelastic_strain_increment,
                        delta_gamma,
                        stress_new,
                        stress_dev,
                        stress_old,
                        elasticity_tensor);
}

Real
ADGeneralizedRadialReturnStressUpdate::computeReferenceResidual(
    const ADDenseVector & /*effective_trial_stress*/,
    const ADDenseVector & /*stress_new*/,
    const ADReal & /*residual*/,
    const ADReal & /*scalar_effective_inelastic_strain*/)
{
  mooseError("ADGeneralizedRadialReturnStressUpdate::computeReferenceResidual must be implemented "
             "by child classes");

  return 0.0;
}

ADReal
ADGeneralizedRadialReturnStressUpdate::maximumPermissibleValue(
    const ADDenseVector & /*effective_trial_stress*/) const
{
  return std::numeric_limits<Real>::max();
}

Real
ADGeneralizedRadialReturnStressUpdate::computeTimeStepLimit()
{

  // Add a new criterion including numerical integration error
  Real scalar_inelastic_strain_incr = MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
                                      _effective_inelastic_strain_old[_qp];

  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
    return std::numeric_limits<Real>::max();

  return std::min(_dt * _max_inelastic_increment / scalar_inelastic_strain_incr,
                  computeIntegrationErrorTimeStep());
}

void
ADGeneralizedRadialReturnStressUpdate::outputIterationSummary(std::stringstream * iter_output,
                                                              const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  }
  ADGeneralizedReturnMappingSolution::outputIterationSummary(iter_output, total_it);
}
