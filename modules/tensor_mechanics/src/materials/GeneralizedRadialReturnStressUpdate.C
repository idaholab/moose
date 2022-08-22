//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedRadialReturnStressUpdate.h"

#include "MooseMesh.h"
#include "MooseTypes.h"
#include "ElasticityTensorTools.h"
#include "libmesh/ignore_warnings.h"
#include "Eigen/Dense"
#include "Eigen/Eigenvalues"
#include "libmesh/restore_warnings.h"

template <bool is_ad>
InputParameters
GeneralizedRadialReturnStressUpdateTempl<is_ad>::validParams()
{
  InputParameters params = StressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription("Calculates the effective inelastic strain increment required to "
                             "return the isotropic stress state to a J2 yield surface.  This class "
                             "is intended to be a parent class for classes with specific "
                             "constitutive models.");
  params += GeneralizedReturnMappingSolutionTempl<is_ad>::validParams();
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
  params.addParam<bool>(
      "use_transformation",
      true,
      "Whether to employ updated Hill's tensor due to rigid body or large "
      "deformation kinematic rotations. If an initial rigid body rotation is provided by the user "
      "in increments of 90 degrees (e.g. 90, 180, 270), this option can be set to false, in which "
      "case the Hill's coefficients are extracted from the transformed Hill's tensor.");

  return params;
}
template <bool is_ad>
GeneralizedRadialReturnStressUpdateTempl<is_ad>::GeneralizedRadialReturnStressUpdateTempl(
    const InputParameters & parameters)
  : StressUpdateBaseTempl<is_ad>(parameters),
    GeneralizedReturnMappingSolutionTempl<is_ad>(parameters),
    _effective_inelastic_strain(this->template declareGenericProperty<Real, is_ad>(
        this->_base_name +
        this->template getParam<std::string>("effective_inelastic_strain_name"))),
    _effective_inelastic_strain_old(this->template getMaterialPropertyOld<Real>(
        this->_base_name +
        this->template getParam<std::string>("effective_inelastic_strain_name"))),
    _inelastic_strain_rate(this->template declareProperty<Real>(
        this->_base_name + this->template getParam<std::string>("inelastic_strain_rate_name"))),
    _inelastic_strain_rate_old(this->template getMaterialPropertyOld<Real>(
        this->_base_name + this->template getParam<std::string>("inelastic_strain_rate_name"))),
    _max_inelastic_increment(this->template getParam<Real>("max_inelastic_increment")),
    _max_integration_error(this->template getParam<Real>("max_integration_error")),
    _max_integration_error_time_step(std::numeric_limits<Real>::max()),
    _use_transformation(this->template getParam<bool>("use_transformation"))
{
}

template <bool is_ad>
void
GeneralizedRadialReturnStressUpdateTempl<is_ad>::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
  _inelastic_strain_rate[_qp] = 0.0;
}

template <bool is_ad>
void
GeneralizedRadialReturnStressUpdateTempl<is_ad>::propagateQpStatefulPropertiesRadialReturn()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
  _inelastic_strain_rate[_qp] = _inelastic_strain_rate_old[_qp];
}

template <bool is_ad>
void
GeneralizedRadialReturnStressUpdateTempl<is_ad>::updateState(
    GenericRankTwoTensor<is_ad> & elastic_strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericRankTwoTensor<is_ad> & /*rotation_increment*/,
    GenericRankTwoTensor<is_ad> & stress_new,
    const RankTwoTensor & stress_old,
    const GenericRankFourTensor<is_ad> & elasticity_tensor,
    const RankTwoTensor & /*elastic_strain_old*/,
    bool /*compute_full_tangent_operator = false*/,
    RankFourTensor & /*tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor*/)
{
  // Prepare initial trial stress for generalized return mapping
  GenericRankTwoTensor<is_ad> deviatoric_trial_stress = stress_new.deviatoric();

  GenericDenseVector<is_ad> stress_new_vector(6);
  stress_new_vector(0) = stress_new(0, 0);
  stress_new_vector(1) = stress_new(1, 1);
  stress_new_vector(2) = stress_new(2, 2);
  stress_new_vector(3) = stress_new(0, 1);
  stress_new_vector(4) = stress_new(1, 2);
  stress_new_vector(5) = stress_new(0, 2);

  GenericDenseVector<is_ad> stress_dev(6);
  stress_dev(0) = deviatoric_trial_stress(0, 0);
  stress_dev(1) = deviatoric_trial_stress(1, 1);
  stress_dev(2) = deviatoric_trial_stress(2, 2);
  stress_dev(3) = deviatoric_trial_stress(0, 1);
  stress_dev(4) = deviatoric_trial_stress(1, 2);
  stress_dev(5) = deviatoric_trial_stress(0, 2);

  computeStressInitialize(stress_dev, stress_new_vector, elasticity_tensor);

  // Use Newton iteration to determine a plastic multiplier variable
  GenericReal<is_ad> delta_gamma = 0.0;

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  if (!MooseUtils::absoluteFuzzyEqual(MetaPhysicL::raw_value(stress_dev).l2_norm(), 0.0))
  {
    this->returnMappingSolve(stress_dev, stress_new_vector, delta_gamma, this->_console);

    computeStrainFinalize(inelastic_strain_increment, stress_new, stress_dev, delta_gamma);
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

template <bool is_ad>
Real
GeneralizedRadialReturnStressUpdateTempl<is_ad>::computeReferenceResidual(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/,
    const GenericDenseVector<is_ad> & /*stress_new*/,
    const GenericReal<is_ad> & /*residual*/,
    const GenericReal<is_ad> & /*scalar_effective_inelastic_strain*/)
{
  mooseError("GeneralizedRadialReturnStressUpdate::computeReferenceResidual must be implemented "
             "by child classes");

  return 0.0;
}

template <bool is_ad>
GenericReal<is_ad>
GeneralizedRadialReturnStressUpdateTempl<is_ad>::maximumPermissibleValue(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/) const
{
  return std::numeric_limits<Real>::max();
}

template <bool is_ad>
Real
GeneralizedRadialReturnStressUpdateTempl<is_ad>::computeTimeStepLimit()
{

  // Add a new criterion including numerical integration error
  Real scalar_inelastic_strain_incr = MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
                                      _effective_inelastic_strain_old[_qp];

  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
    return std::numeric_limits<Real>::max();

  return std::min(_dt * _max_inelastic_increment / scalar_inelastic_strain_incr,
                  computeIntegrationErrorTimeStep());
}

template <bool is_ad>
void
GeneralizedRadialReturnStressUpdateTempl<is_ad>::outputIterationSummary(
    std::stringstream * iter_output, const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  }
  GeneralizedReturnMappingSolutionTempl<is_ad>::outputIterationSummary(iter_output, total_it);
}

template <bool is_ad>
bool
GeneralizedRadialReturnStressUpdateTempl<is_ad>::isBlockDiagonal(const AnisotropyMatrixReal & A)
{
  AnisotropyMatrixRealBlock A_bottom_left(A.block<3, 3>(0, 3));
  AnisotropyMatrixRealBlock A_top_right(A.block<3, 3>(3, 0));

  for (unsigned int index_i = 0; index_i < 3; index_i++)
    for (unsigned int index_j = 0; index_j < 3; index_j++)
      if (A_bottom_left(index_i, index_j) != 0 || A_top_right(index_i, index_j) != 0)
        return false;

  return true;
}

template class GeneralizedRadialReturnStressUpdateTempl<false>;
template class GeneralizedRadialReturnStressUpdateTempl<true>;
