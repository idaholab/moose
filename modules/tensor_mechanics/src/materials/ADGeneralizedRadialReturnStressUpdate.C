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
  params.addParam<RealVectorValue>("rotation_angles",
                                   "Provide the rotation angles for the transformation matrix. "
                                   "This should be a vector that provides "
                                   "the rotation angles about z-, y-, and x-axis, respectively.");
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
    _max_integration_error_time_step(std::numeric_limits<Real>::max()),
    _zyx_angles(isParamValid("rotation_angles") ? getParam<RealVectorValue>("rotation_angles")
                                                : RealVectorValue(0.0, 0.0, 0.0)),
    _transformation_tensor(6, 6),
    _hill_constants(6)

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

void
ADGeneralizedRadialReturnStressUpdate::rotateHillConstants(std::vector<Real> & hill_constants_input)
{
  const Real sz = std::sin(_zyx_angles(0) * libMesh::pi / 180.0);
  const Real cz = std::cos(_zyx_angles(0) * libMesh::pi / 180.0);

  const Real sy = std::sin(_zyx_angles(1) * libMesh::pi / 180.0);
  const Real cy = std::cos(_zyx_angles(1) * libMesh::pi / 180.0);

  const Real sx = std::sin(_zyx_angles(2) * libMesh::pi / 180.0);
  const Real cx = std::cos(_zyx_angles(2) * libMesh::pi / 180.0);

  // transformation matrix is formed by performing the ZYX rotation
  _transformation_tensor(0, 0) = cy * cy * cz * cz;
  _transformation_tensor(0, 1) = sz * sz * cy * cy;
  _transformation_tensor(0, 2) = sy * sy;
  _transformation_tensor(0, 3) = -2.0 * sy * sz * cy;
  _transformation_tensor(0, 4) = 2.0 * sy * cy * cz;
  _transformation_tensor(0, 5) = 2.0 * sz * cy * cy * cz;

  _transformation_tensor(1, 0) =
      sx * sx * sy * sy * cz * cz + 2.0 * sx * sy * sz * cx * cz + sz * sz * cx * cx;
  _transformation_tensor(1, 1) =
      sx * sx * sz * sz * sy * sy - 2.0 * sx * sy * sz * cx * cz + cx * cx * cz * cz;
  _transformation_tensor(1, 2) = sx * sx * cy * cy;
  _transformation_tensor(1, 3) = 2.0 * sx * sx * sz * sy * cy + 2.0 * sx * cx * cy * cz;
  _transformation_tensor(1, 4) = -2.0 * sx * sx * sy * cy * cz + 2.0 * sx * sz * cx * cz;
  _transformation_tensor(1, 5) = -2.0 * (-sz * sz + cz * cz) * sx * sy * cx +
                                 2.0 * sx * sx * sy * sy * sz * cz - 2.0 * sz * cx * cx * cz;

  _transformation_tensor(2, 0) =
      sx * sx * sz * sz - 2.0 * sx * sy * sz * cx * cz + sy * sy * cx * cx * cz * cz;
  _transformation_tensor(2, 1) =
      sx * sx * cz * cz + 2.0 * sx * sy * sz * cx * cz + sy * sy * sz * sz * cx * cx;
  _transformation_tensor(2, 2) = cx * cx * cy * cy;
  _transformation_tensor(2, 3) = -2.0 * sx * cx * cy * cz + 2.0 * sy * sz * cx * cx * cy;
  _transformation_tensor(2, 4) = -2.0 * sx * sz * cx * cy - 2.0 * sy * cx * cx * cy * cz;
  _transformation_tensor(2, 5) = 2.0 * (-sz * sz + cz * cz) * sx * sy * cx -
                                 2.0 * sx * sx * sz * cz + 2.0 * sy * sy * sz * cx * cx * cz;

  _transformation_tensor(3, 0) =
      (-sx * sx + cx * cx) * sy * sz * cz + sx * sy * sy * cx * cz * cz - sx * sz * sz * cx;
  _transformation_tensor(3, 1) =
      -(-sx * sx + cx * cx) * sy * sz * cz + sx * sy * sy * sz * sz * cx - sx * cx * cz * cz;
  _transformation_tensor(3, 2) = sx * cx * cy * cy;
  _transformation_tensor(3, 3) = (-sx * sx + cx * cx) * cy * cz + 2.0 * sx * sy * sz * cx * cy;
  _transformation_tensor(3, 4) = (-sx * sx + cx * cx) * sz * cy - 2.0 * sx * sy * cx * cy * cz;
  _transformation_tensor(3, 5) = -(-sx * sx + cx * cx) * (-sz * sz + cz * cz) * sy +
                                 2.0 * sx * sy * sy * sz * cx * cz + 2.0 * sx * sz * cx * cz;

  _transformation_tensor(4, 0) = sx * sz * cy * cz - sy * cx * cy * cz * cz;
  _transformation_tensor(4, 1) = -sx * sz * cy * cz - sy * sz * sz * cx * cy;
  _transformation_tensor(4, 2) = sy * cx * cy;
  _transformation_tensor(4, 3) = -(-sy * sy + cy * cy) * sz * cx - sx * sy * cz;
  _transformation_tensor(4, 4) = (-sy * sy + cy * cy) * cx * cz - sx * sy * sz;
  _transformation_tensor(4, 5) = -(-sz * sz + cz * cz) * sx * cy - 2.0 * sy * sz * cx * cy * cz;

  _transformation_tensor(5, 0) = -sx * sy * cy * cz * cz - sz * cx * cy * cz;
  _transformation_tensor(5, 1) = -sx * sy * sz * sz * cy + sz * cx * cy * cz;
  _transformation_tensor(5, 2) = sx * sy * cy;
  _transformation_tensor(5, 3) = -(-sy * sy + cy * cy) * sx * sz + sy * cx * cz;
  _transformation_tensor(5, 4) = (-sy * sy + cy * cy) * sx * cz + sy * sz * cx;
  _transformation_tensor(5, 5) = (-sz * sz + cz * cz) * cx * cy - 2.0 * sx * sy * sz * cy * cz;

  // store hill constants
  const Real & F = hill_constants_input[0];
  const Real & G = hill_constants_input[1];
  const Real & H = hill_constants_input[2];
  const Real & L = hill_constants_input[3];
  const Real & M = hill_constants_input[4];
  const Real & N = hill_constants_input[5];

  // rotated hill constants are calculated from rotated hill tensor, Hill_rot = Tm*Hill*Tm^T
  _hill_constants[0] = -_transformation_tensor(1, 0) *
                           (-G * _transformation_tensor(2, 2) - H * _transformation_tensor(2, 1) +
                            _transformation_tensor(2, 0) * (G + H)) -
                       _transformation_tensor(1, 1) *
                           (-F * _transformation_tensor(2, 2) - H * _transformation_tensor(2, 0) +
                            _transformation_tensor(2, 1) * (F + H)) -
                       _transformation_tensor(1, 2) *
                           (-F * _transformation_tensor(2, 1) - G * _transformation_tensor(2, 0) +
                            _transformation_tensor(2, 2) * (F + G)) -
                       2.0 * L * _transformation_tensor(1, 4) * _transformation_tensor(2, 4) -
                       2.0 * M * _transformation_tensor(1, 5) * _transformation_tensor(2, 5) -
                       2.0 * N * _transformation_tensor(1, 3) * _transformation_tensor(2, 3);

  _hill_constants[1] = -_transformation_tensor(0, 0) *
                           (-G * _transformation_tensor(2, 2) - H * _transformation_tensor(2, 1) +
                            _transformation_tensor(2, 0) * (G + H)) -
                       _transformation_tensor(0, 1) *
                           (-F * _transformation_tensor(2, 2) - H * _transformation_tensor(2, 0) +
                            _transformation_tensor(2, 1) * (F + H)) -
                       _transformation_tensor(0, 2) *
                           (-F * _transformation_tensor(2, 1) - G * _transformation_tensor(2, 0) +
                            _transformation_tensor(2, 2) * (F + G)) -
                       2.0 * L * _transformation_tensor(0, 4) * _transformation_tensor(2, 4) -
                       2.0 * M * _transformation_tensor(0, 5) * _transformation_tensor(2, 5) -
                       2.0 * N * _transformation_tensor(0, 3) * _transformation_tensor(2, 3);

  _hill_constants[2] = -_transformation_tensor(0, 0) *
                           (-G * _transformation_tensor(1, 2) - H * _transformation_tensor(1, 1) +
                            _transformation_tensor(1, 0) * (G + H)) -
                       _transformation_tensor(0, 1) *
                           (-F * _transformation_tensor(1, 2) - H * _transformation_tensor(1, 0) +
                            _transformation_tensor(1, 1) * (F + H)) -
                       _transformation_tensor(0, 2) *
                           (-F * _transformation_tensor(1, 1) - G * _transformation_tensor(1, 0) +
                            _transformation_tensor(1, 2) * (F + G)) -
                       2.0 * L * _transformation_tensor(0, 4) * _transformation_tensor(1, 4) -
                       2.0 * M * _transformation_tensor(0, 5) * _transformation_tensor(1, 5) -
                       2.0 * N * _transformation_tensor(0, 3) * _transformation_tensor(1, 3);

  _hill_constants[3] = 0.5 * _transformation_tensor(4, 0) *
                           (-G * _transformation_tensor(4, 2) - H * _transformation_tensor(4, 1) +
                            _transformation_tensor(4, 0) * (G + H)) +
                       0.5 * _transformation_tensor(4, 1) *
                           (-F * _transformation_tensor(4, 2) - H * _transformation_tensor(4, 0) +
                            _transformation_tensor(4, 1) * (F + H)) +
                       0.5 * _transformation_tensor(4, 2) *
                           (-F * _transformation_tensor(4, 1) - G * _transformation_tensor(4, 0) +
                            _transformation_tensor(4, 2) * (F + G)) +
                       L * _transformation_tensor(4, 4) * _transformation_tensor(4, 4) +
                       M * _transformation_tensor(4, 5) * _transformation_tensor(4, 5) +
                       N * _transformation_tensor(4, 3) * _transformation_tensor(4, 3);

  _hill_constants[4] = 0.5 * _transformation_tensor(5, 0) *
                           (-G * _transformation_tensor(5, 2) - H * _transformation_tensor(5, 1) +
                            _transformation_tensor(5, 0) * (G + H)) +
                       0.5 * _transformation_tensor(5, 1) *
                           (-F * _transformation_tensor(5, 2) - H * _transformation_tensor(5, 0) +
                            _transformation_tensor(5, 1) * (F + H)) +
                       0.5 * _transformation_tensor(5, 2) *
                           (-F * _transformation_tensor(5, 1) - G * _transformation_tensor(5, 0) +
                            _transformation_tensor(5, 2) * (F + G)) +
                       L * _transformation_tensor(5, 4) * _transformation_tensor(5, 4) +
                       M * _transformation_tensor(5, 5) * _transformation_tensor(5, 5) +
                       N * _transformation_tensor(5, 3) * _transformation_tensor(5, 3);

  _hill_constants[5] = 0.5 * _transformation_tensor(3, 0) *
                           (-G * _transformation_tensor(3, 2) - H * _transformation_tensor(3, 1) +
                            _transformation_tensor(3, 0) * (G + H)) +
                       0.5 * _transformation_tensor(3, 1) *
                           (-F * _transformation_tensor(3, 2) - H * _transformation_tensor(3, 0) +
                            _transformation_tensor(3, 1) * (F + H)) +
                       0.5 * _transformation_tensor(3, 2) *
                           (-F * _transformation_tensor(3, 1) - G * _transformation_tensor(3, 0) +
                            _transformation_tensor(3, 2) * (F + G)) +
                       L * _transformation_tensor(3, 4) * _transformation_tensor(3, 4) +
                       M * _transformation_tensor(3, 5) * _transformation_tensor(3, 5) +
                       N * _transformation_tensor(3, 3) * _transformation_tensor(3, 3);
}
