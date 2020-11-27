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
#include <Eigen/Dense>

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
  params.addRequiredParam<std::string>(
      "effective_inelastic_strain_name",
      "Name of the material property that stores the effective inelastic strain");
  params.addParam<bool>("apply_strain", false, "Flag to apply strain. Used for testing.");
  params.addParamNamesToGroup("effective_inelastic_strain_name apply_strain", "Advanced");
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
    _max_inelastic_increment(getParam<Real>("max_inelastic_increment")),
    _apply_strain(getParam<bool>("apply_strain"))
{
}

void
ADGeneralizedRadialReturnStressUpdate::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
}

void
ADGeneralizedRadialReturnStressUpdate::propagateQpStatefulPropertiesRadialReturn()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
}

void
ADGeneralizedRadialReturnStressUpdate::updateState(ADRankTwoTensor & elastic_strain_increment,
                                                   ADRankTwoTensor & inelastic_strain_increment,
                                                   const ADRankTwoTensor & /*rotation_increment*/,
                                                   ADRankTwoTensor & stress_new,
                                                   const RankTwoTensor & /*stress_old*/,
                                                   const ADRankFourTensor & elasticity_tensor,
                                                   const RankTwoTensor & /*elastic_strain_old*/)
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

  //  ADDenseMatrix rotation_matrix_transpose(6, 6);
  //  _eigenvectors_hill.get_transpose(rotation_matrix_transpose);
  //
  //  ADDenseVector stress_dev_hat(6);
  //  rotation_matrix_transpose.vector_mult(stress_dev_hat, stress_dev);

  computeStressInitialize(stress_dev, elasticity_tensor);

  // Use Newton iteration to determine a plastic multiplier variable
  ADReal delta_gamma = 0.0;
  // Set the value of 3 * shear modulus for use as a reference residual value
  _two_shear_modulus = 2.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  if (!MooseUtils::absoluteFuzzyEqual(MetaPhysicL::raw_value(stress_dev).l2_norm(), 0.0))
  {
    returnMappingSolve(stress_dev, stress_new_vector, delta_gamma, _console);

    if (delta_gamma != 0.0)
      computeStrainFinalize(inelastic_strain_increment, stress_new, delta_gamma);
    else
      inelastic_strain_increment.zero();
    //    if (_qp == 0)
    //    {
    //      Moose::out << "inelastic_strain_increment: "
    //                 << MetaPhysicL::raw_value(inelastic_strain_increment) << "\n";
    //      Moose::out << "strain_increment: " << MetaPhysicL::raw_value(elastic_strain_increment)
    //                 << "\n";
    //    }
  }
  else
    inelastic_strain_increment.zero();

  elastic_strain_increment -= inelastic_strain_increment;
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp] + delta_gamma;
  computeStressFinalize(inelastic_strain_increment, delta_gamma, stress_new);
  //  if (_qp == 0)
  //  {
  //    Moose::out << "End of RM: inelastic_strain_increment: "
  //               << MetaPhysicL::raw_value(inelastic_strain_increment) << "\n";
  //    Moose::out << "End of RM: strain_increment: "
  //               << MetaPhysicL::raw_value(elastic_strain_increment) << "\n";
  //  }
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
  Real scalar_inelastic_strain_incr = MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
                                      _effective_inelastic_strain_old[_qp];

  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
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
