//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnisotropicReturnStressUpdate.h"

#include "MooseMesh.h"
#include "ElasticityTensorTools.h"

InputParameters
AnisotropicReturnStressUpdate::validParams()
{
  InputParameters params = StressUpdateBase::validParams();
  params.addClassDescription("Calculates the effective inelastic strain increment required to "
                             "return the isotropic stress state to a J2 yield surface.  This class "
                             "is intended to be a parent class for classes with specific "
                             "constitutive models.");
  params += MultiVariableReturnMappingSolution::validParams();
  params.addParam<Real>("max_inelastic_increment",
                        1e-4,
                        "The maximum inelastic strain increment allowed in a time step");
  params.addRequiredParam<std::string>(
      "effective_inelastic_strain_name",
      "Name of the material property that stores the effective inelastic strain");
  params.addParamNamesToGroup("effective_inelastic_strain_name", "Advanced");
  params.addRequiredParam<std::vector<Real>>("hill_constants",
                                             "Hill material constants in order: F, "
                                             "G, H, L, M, N");

  return params;
}

AnisotropicReturnStressUpdate::AnisotropicReturnStressUpdate(const InputParameters & parameters)
  : StressUpdateBase(parameters),
    MultiVariableReturnMappingSolution(parameters),
    _effective_inelastic_strain(declareProperty<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _effective_inelastic_strain_old(getMaterialPropertyOld<Real>(
        _base_name + getParam<std::string>("effective_inelastic_strain_name"))),
    _max_inelastic_increment(parameters.get<Real>("max_inelastic_increment")),
    _identity_two(RankTwoTensor::initIdentity),
    _identity_symmetric_four(RankFourTensor::initIdentitySymmetricFour),
    _deviatoric_projection_four(_identity_symmetric_four -
                                _identity_two.outerProduct(_identity_two) / 3.0),
    _hill_constants(6),
    _eigenvalues_hill(6),
    _eigenvectors_hill(6, 6)
{
  // Hill constants, some constraints apply
  const Real F = _hill_constants[0];
  const Real G = _hill_constants[1];
  const Real H = _hill_constants[2];
  const Real L = _hill_constants[3];
  const Real M = _hill_constants[4];
  const Real N = _hill_constants[5];

  DenseMatrix<Real> hill_tensor(6, 6);
  hill_tensor(0, 0) = G + H;
  hill_tensor(1, 1) = F + H;
  hill_tensor(2, 2) = F + G;
  hill_tensor(0, 1) = hill_tensor(1, 0) = -H;
  hill_tensor(0, 2) = hill_tensor(2, 0) = -G;
  hill_tensor(1, 2) = hill_tensor(2, 1) = -F;

  hill_tensor(3, 3) = 2.0 * N;
  hill_tensor(4, 4) = 2.0 * L;
  hill_tensor(5, 5) = 2.0 * M;

  computeHillTensorEigenDecomposition(hill_tensor);
}

void
AnisotropicReturnStressUpdate::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0;
}

void
AnisotropicReturnStressUpdate::propagateQpStatefulPropertiesRadialReturn()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
}

void
AnisotropicReturnStressUpdate::updateState(RankTwoTensor & strain_increment,
                                           RankTwoTensor & inelastic_strain_increment,
                                           const RankTwoTensor & /*rotation_increment*/,
                                           RankTwoTensor & stress_new,
                                           const RankTwoTensor & /*stress_old*/,
                                           const RankFourTensor & elasticity_tensor,
                                           const RankTwoTensor & elastic_strain_old,
                                           bool compute_full_tangent_operator,
                                           RankFourTensor & tangent_operator)
{
  // compute the deviatoric trial stress and trial strain from the current intermediate
  // configuration
  RankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  // compute the effective trial stress
  Real dev_trial_stress_squared =
      deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress);

  // TODO FIXME: What's the effective trial stress tensor?
  // Trial stress is simply the deviatoric part of current stress (stress_new)
  RankTwoTensor effective_trial_stress = stress_new.deviatoric();

  // Set the value of 3 * shear modulus for use as a reference residual value
  _three_shear_modulus = 3.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  computeStressInitialize(effective_trial_stress, elasticity_tensor);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  Real scalar_effective_inelastic_strain = 0.0;
  RankTwoTensor zero_tensor(RankTwoTensor::initNone);

  if (!(effective_trial_stress == zero_tensor)) // There was a fuzzy equal here FIXME
  {
    returnMappingSolve(effective_trial_stress, scalar_effective_inelastic_strain, _console);
    if (scalar_effective_inelastic_strain != 0.0)
      inelastic_strain_increment =
          deviatoric_trial_stress *
          (1.5 * scalar_effective_inelastic_strain / effective_trial_stress.L2norm()); // FIXME
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
  stress_new = elasticity_tensor * (strain_increment + elastic_strain_old);

  computeStressFinalize(inelastic_strain_increment);

  if (compute_full_tangent_operator &&
      getTangentCalculationMethod() == TangentCalculationMethod::PARTIAL)
  {
    if (MooseUtils::absoluteFuzzyEqual(scalar_effective_inelastic_strain, 0.0))
      tangent_operator.zero();
    else
    {
      // mu = _three_shear_modulus / 3.0;
      // norm_dev_stress = ||s_n+1||
      // effective_trial_stress = von mises trial stress = std::sqrt(3.0 / 2.0) * ||s_n+1^trial||
      // scalar_effective_inelastic_strain = Delta epsilon^cr_n+1
      // deriv = derivative of scalar_effective_inelastic_strain w.r.t. von mises stress
      // deriv = std::sqrt(3.0 / 2.0) partial Delta epsilon^cr_n+1n over partial ||s_n+1^trial||

      mooseAssert(_three_shear_modulus != 0.0, "Shear modulus is zero");

      const RankTwoTensor deviatoric_stress = stress_new.deviatoric();
      const Real norm_dev_stress =
          std::sqrt(deviatoric_stress.doubleContraction(deviatoric_stress));
      mooseAssert(norm_dev_stress != 0.0, "Norm of the deviatoric is zero");

      const RankTwoTensor flow_direction = deviatoric_stress / norm_dev_stress;
      const RankFourTensor flow_direction_dyad = flow_direction.outerProduct(flow_direction);
      const Real deriv =
          computeStressDerivative(effective_trial_stress, scalar_effective_inelastic_strain);
      const Real scalar_one = _three_shear_modulus * scalar_effective_inelastic_strain /
                              std::sqrt(1.5) / norm_dev_stress;

      tangent_operator = scalar_one * _deviatoric_projection_four +
                         (_three_shear_modulus * deriv - scalar_one) * flow_direction_dyad;
    }
  }
}

Real
AnisotropicReturnStressUpdate::computeReferenceResidual(
    const RankTwoTensor effective_trial_stress, const Real scalar_effective_inelastic_strain)
{
  // FIXME: Convert to tensor form
  return effective_trial_stress.L2norm() / _three_shear_modulus - scalar_effective_inelastic_strain;
}

Real
AnisotropicReturnStressUpdate::maximumPermissibleValue(
    const RankTwoTensor effective_trial_stress) const
{
  // FIXME: Convert to tensor form
  return effective_trial_stress.L2norm() / _three_shear_modulus;
}

Real
AnisotropicReturnStressUpdate::computeTimeStepLimit()
{
  Real scalar_inelastic_strain_incr;

  scalar_inelastic_strain_incr =
      _effective_inelastic_strain[_qp] - _effective_inelastic_strain_old[_qp];
  if (MooseUtils::absoluteFuzzyEqual(scalar_inelastic_strain_incr, 0.0))
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}

void
AnisotropicReturnStressUpdate::computeHillTensorEigenDecomposition(DenseMatrix<Real> & hill_tensor)
{
  unsigned int dimension = hill_tensor.n();

  // Initializing temporary objects for the eigenvalues and eigenvectors since
  // evd_left() returns an unordered vector of eigenvalues.
  DenseVector<Real> eigenvalues(dimension);
  DenseMatrix<Real> eigenvectors(dimension, dimension);

  // Creating a temporary placeholder for the imaginary parts of the eigenvalues
  DenseVector<Real> eigenvalues_imag(dimension);

  // Performing the eigenvalue decomposition
  hill_tensor.evd_left(eigenvalues, eigenvalues_imag, eigenvectors);

  _eigenvalues_hill = eigenvalues;
  _eigenvectors_hill = eigenvectors;

  // printEigenvalues();
}

void
AnisotropicReturnStressUpdate::outputIterationSummary(std::stringstream * iter_output,
                                                      const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  }
  MultiVariableReturnMappingSolution::outputIterationSummary(iter_output, total_it);
}
