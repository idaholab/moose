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
  // Prepare initial trial stress for generalized return mapping
  RankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  DenseVector<Real> stress_new_vector(6);
  stress_new_vector(0) = stress_new(0, 0);
  stress_new_vector(1) = stress_new(1, 1);
  stress_new_vector(2) = stress_new(2, 2);
  stress_new_vector(3) = stress_new(0, 1);
  stress_new_vector(4) = stress_new(1, 2);
  stress_new_vector(5) = stress_new(0, 2);

  DenseVector<Real> stress_dev(6);
  stress_dev(0) = deviatoric_trial_stress(0, 0);
  stress_dev(1) = deviatoric_trial_stress(1, 1);
  stress_dev(2) = deviatoric_trial_stress(2, 2);
  stress_dev(3) = deviatoric_trial_stress(0, 1);
  stress_dev(4) = deviatoric_trial_stress(1, 2);
  stress_dev(5) = deviatoric_trial_stress(0, 2);

  DenseMatrix<Real> rotation_matrix_transpose(6, 6);
  _eigenvectors_hill.get_transpose(rotation_matrix_transpose);

  DenseVector<Real> stress_dev_hat(6);
  rotation_matrix_transpose.vector_mult(stress_dev_hat, stress_dev);

  computeStressInitialize(stress_dev_hat, elasticity_tensor);

  // Use Newton iteration to determine a plastic multiplier variable
  Real delta_gamma = 0.0;

  if (!(MooseUtils::absoluteFuzzyEqual(stress_dev_hat.l2_norm(), 0.0)))
  {
    // Call "Multi" variable return mapping (and therefore its material models)
    returnMappingSolve(stress_dev_hat, stress_new_vector, delta_gamma, _console);

    // What are we doing here? Not sure if we need this
    if (delta_gamma != 0.0)
      computeStrainFinalize(inelastic_strain_increment, stress_new, delta_gamma);
    else
      inelastic_strain_increment.zero();
  }

  strain_increment -= inelastic_strain_increment;

  // For creep update, delta_gamma is a creep inelastic strain increment
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp] + delta_gamma;

  // Use the old elastic strain here because we require tensors used by this class
  // to be isotropic and this method natively allows for changing in time
  // elasticity tensors
  // TODO: ROTATE ELASTICITY TENSOR PROPERLY. FIXME
  stress_new = elasticity_tensor * (strain_increment + elastic_strain_old);

  computeStressFinalize(inelastic_strain_increment);

  //  if (compute_full_tangent_operator &&
  //      getTangentCalculationMethod() == TangentCalculationMethod::PARTIAL)
  //  {
  //    if (MooseUtils::absoluteFuzzyEqual(delta_gamma, 0.0))
  //      tangent_operator.zero();
  //    else
  //    {
  //      // mu = _three_shear_modulus / 3.0;
  //      // norm_dev_stress = ||s_n+1||
  //      // effective_trial_stress = von mises trial stress = std::sqrt(3.0 / 2.0) * ||s_n+1^trial||
  //      // scalar_effective_inelastic_strain = Delta epsilon^cr_n+1
  //      // deriv = derivative of scalar_effective_inelastic_strain w.r.t. von mises stress
  //      // deriv = std::sqrt(3.0 / 2.0) partial Delta epsilon^cr_n+1n over partial ||s_n+1^trial||
  //
  //      mooseAssert(_three_shear_modulus != 0.0, "Shear modulus is zero");
  //
  //      const RankTwoTensor deviatoric_stress = stress_new.deviatoric();
  //      const Real norm_dev_stress =
  //          std::sqrt(deviatoric_stress.doubleContraction(deviatoric_stress));
  //      mooseAssert(norm_dev_stress != 0.0, "Norm of the deviatoric is zero");
  //
  //      const RankTwoTensor flow_direction = deviatoric_stress / norm_dev_stress;
  //      const RankFourTensor flow_direction_dyad = flow_direction.outerProduct(flow_direction);
  //      const Real deriv = computeStressDerivative(stress_dev_hat, stress_new_vector,
  //      delta_gamma); const Real scalar_one = _three_shear_modulus * delta_gamma / std::sqrt(1.5)
  //      / norm_dev_stress;
  //
  //      tangent_operator = scalar_one * _deviatoric_projection_four +
  //                         (_three_shear_modulus * deriv - scalar_one) * flow_direction_dyad;
  //    }
  //  }
}

Real
AnisotropicReturnStressUpdate::maximumPermissibleValue(
    const DenseVector<Real> & /*effective_trial_stress*/) const
{
  return std::numeric_limits<Real>::max();
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
