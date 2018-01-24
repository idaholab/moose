//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeSmearedCrackingStress.h"
#include "ElasticityTensorTools.h"
#include "StressUpdateBase.h"

template <>
InputParameters
validParams<ComputeSmearedCrackingStress>()
{
  InputParameters params = validParams<ComputeMultipleInelasticStress>();
  params.addClassDescription("Compute stress using a fixed smeared cracking model");
  params.addParam<std::string>(
      "cracking_release",
      "abrupt",
      "The cracking release type.  Choices are abrupt (default) and exponential.");
  params.addParam<Real>(
      "cracking_residual_stress",
      0.0,
      "The fraction of the cracking stress allowed to be maintained following a crack.");
  params.addRequiredCoupledVar(
      "cracking_stress",
      "The stress threshold beyond which cracking occurs. Negative values prevent cracking.");
  params.addParam<std::vector<unsigned int>>(
      "active_crack_planes", "Planes on which cracks are allowed (0,1,2 -> x,z,theta in RZ)");
  params.addParam<unsigned int>(
      "max_cracks", 3, "The maximum number of cracks allowed at a material point.");
  params.addParam<Real>("cracking_neg_fraction",
                        0,
                        "The fraction of the cracking strain at which "
                        "a transitition begins during decreasing "
                        "strain to the original stiffness.");
  params.addParam<Real>("cracking_beta",
                        1.0,
                        "Coefficient used to control the softening in the exponential model.  "
                        "When set to 1, the initial softening slope is equal to the negative "
                        "of the Young's modulus.  Smaller numbers scale down that slope.");
  params.addParam<Real>(
      "max_stress_correction",
      1.0,
      "Maximum permitted correction to the predicted stress as a ratio of the "
      "stress change to the predicted stress from the previous step's damage level. "
      "Values less than 1 will improve robustness, but not be as accurate.");

  params.addRangeCheckedParam<Real>(
      "shear_retention_factor",
      0.0,
      "shear_retention_factor>=0 & shear_retention_factor<=1.0",
      "Fraction of original shear stiffness to be retained after cracking");
  params.set<std::vector<MaterialName>>("inelastic_models") = {};

  return params;
}

namespace
{
ComputeSmearedCrackingStress::CRACKING_RELEASE
getCrackingModel(const std::string & name)
{
  std::string n(name);
  std::transform(n.begin(), n.end(), n.begin(), ::tolower);
  ComputeSmearedCrackingStress::CRACKING_RELEASE cm(ComputeSmearedCrackingStress::CR_UNKNOWN);
  if (n == "abrupt")
    cm = ComputeSmearedCrackingStress::CR_ABRUPT;
  else if (n == "exponential")
    cm = ComputeSmearedCrackingStress::CR_EXPONENTIAL;
  else if (n == "power")
    cm = ComputeSmearedCrackingStress::CR_POWER;
  if (cm == ComputeSmearedCrackingStress::CR_UNKNOWN)
    mooseError("Unknown cracking model");
  return cm;
}
}

ComputeSmearedCrackingStress::ComputeSmearedCrackingStress(const InputParameters & parameters)
  : ComputeMultipleInelasticStress(parameters),
    _cracking_release(getCrackingModel(getParam<std::string>("cracking_release"))),
    _cracking_residual_stress(getParam<Real>("cracking_residual_stress")),
    _cracking_stress(coupledValue("cracking_stress")),
    _active_crack_planes(3, 1),
    _max_cracks(getParam<unsigned int>("max_cracks")),
    _cracking_neg_fraction(getParam<Real>("cracking_neg_fraction")),
    _cracking_beta(getParam<Real>("cracking_beta")),
    _shear_retention_factor(getParam<Real>("shear_retention_factor")),
    _max_stress_correction(getParam<Real>("max_stress_correction")),
    _crack_flags(declareProperty<RealVectorValue>(_base_name + "crack_flags")),
    _crack_flags_old(getMaterialPropertyOld<RealVectorValue>(_base_name + "crack_flags")),
    _crack_count(NULL),
    _crack_count_old(NULL),
    _crack_rotation(declareProperty<RankTwoTensor>(_base_name + "crack_rotation")),
    _crack_rotation_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "crack_rotation")),
    _crack_strain(declareProperty<RealVectorValue>(_base_name + "crack_strain")),
    _crack_strain_old(getMaterialPropertyOld<RealVectorValue>(_base_name + "crack_strain")),
    _crack_max_strain(declareProperty<RealVectorValue>(_base_name + "crack_max_strain")),
    _crack_max_strain_old(getMaterialPropertyOld<RealVectorValue>(_base_name + "crack_max_strain"))
{
  if (_cracking_release == CR_POWER)
  {
    _crack_count = &declareProperty<RealVectorValue>(_base_name + "crack_count");
    _crack_count_old = &getMaterialPropertyOld<RealVectorValue>(_base_name + "crack_count");
  }

  if (parameters.isParamValid("active_crack_planes"))
  {
    const std::vector<unsigned int> & planes =
        getParam<std::vector<unsigned>>("active_crack_planes");
    for (unsigned int i = 0; i < 3; ++i)
      _active_crack_planes[i] = 0;

    for (unsigned int i = 0; i < planes.size(); ++i)
    {
      if (planes[i] > 2)
        mooseError("Active planes must be 0, 1, or 2");
      _active_crack_planes[planes[i]] = 1;
    }
  }
  if (_cracking_residual_stress < 0 || _cracking_residual_stress > 1)
    mooseError("cracking_residual_stress must be between 0 and 1");
  if (parameters.isParamSetByUser("cracking_neg_fraction") &&
      (_cracking_neg_fraction <= 0 || _cracking_neg_fraction > 1))
    mooseError("cracking_neg_fraction must be > zero and <= 1");
}

void
ComputeSmearedCrackingStress::initQpStatefulProperties()
{
  ComputeMultipleInelasticStress::initQpStatefulProperties();

  _crack_flags[_qp](0) = _crack_flags[_qp](1) = _crack_flags[_qp](2) = 1;

  if (_crack_count)
    (*_crack_count)[_qp] = 0;

  _crack_strain[_qp] = 0;
  _crack_max_strain[_qp](0) = 0;
  _crack_rotation[_qp].Identity();
}

void
ComputeSmearedCrackingStress::initialSetup()
{
  ComputeMultipleInelasticStress::initialSetup();
  _is_finite_strain = hasBlockMaterialProperty<RankTwoTensor>(_base_name + "strain_increment");

  if (!hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError("ComputeSmearedCrackingStress requires that the elasticity tensor be "
               "guaranteed isotropic");
}

void
ComputeSmearedCrackingStress::computeQpStress()
{
  bool force_elasticity_rotation = false;

  if (!previouslyCracked())
    computeQpStressIntermediateConfiguration();
  else
  {
    _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp];

    // Propagate behavior from the (now inactive) inelastic models
    _inelastic_strain[_qp] = _inelastic_strain_old[_qp];
    for (auto model : _models)
    {
      model->setQp(_qp);
      model->propagateQpStatefulProperties();
    }

    // Since the other inelastic models are inactive, they will not limit the time step
    _matl_timestep_limit[_qp] = std::numeric_limits<Real>::max();

    // update elasticity tensor with old cracking status: crack_flags_old and crack_orientation_old
    updateElasticityTensor();

    // Calculate stress in intermediate configuration
    _stress[_qp] = _local_elasticity_tensor * _elastic_strain[_qp];
    // InitialStress Deprecation: remove these lines
    if (_perform_finite_strain_rotations)
      rotateQpInitialStress();
    addQpInitialStress();

    _Jacobian_mult[_qp] = _local_elasticity_tensor;
    force_elasticity_rotation = true;
  }

  // compute crack status and adjust stress
  crackingStressRotation();

  if (_perform_finite_strain_rotations)
  {
    finiteStrainRotation(force_elasticity_rotation);
    _crack_rotation[_qp] =
        _rotation_increment[_qp] * _crack_rotation[_qp] * _rotation_increment[_qp].transpose();
  }
}

void
ComputeSmearedCrackingStress::updateElasticityTensor()
{
  const Real youngs_modulus =
      ElasticityTensorTools::getIsotropicYoungsModulus(_elasticity_tensor[_qp]);

  bool cracking_locally_active = false;

  const Real cracking_stress = _cracking_stress[_qp];

  if (cracking_stress > 0)
  {
    RealVectorValue crack_flags_local(1.0, 1.0, 1.0);
    const RankTwoTensor & R = _crack_rotation_old[_qp];
    RankTwoTensor ePrime(_elastic_strain_old[_qp]);
    ePrime.rotate(R.transpose());

    for (unsigned int i = 0; i < 3; ++i)
    {
      // Update elasticity tensor based on crack status of the end of last time step
      if (_crack_flags_old[_qp](i) < 1.0)
      {
        if (_cracking_neg_fraction == 0 && MooseUtils::absoluteFuzzyLessThan(ePrime(i, i), 0.0))
          crack_flags_local(i) = 1.0;
        else if (_cracking_neg_fraction > 0 &&
                 ePrime(i, i) < _crack_strain_old[_qp](i) * _cracking_neg_fraction &&
                 ePrime(i, i) > -_crack_strain_old[_qp](i) * _cracking_neg_fraction)
        {
          const Real etr = _cracking_neg_fraction * _crack_strain_old[_qp](i);
          const Real Eo = cracking_stress / _crack_strain_old[_qp](i);
          const Real Ec = Eo * _crack_flags_old[_qp](i);
          const Real a = (Ec - Eo) / (4 * etr);
          const Real b = (Ec + Eo) / 2;
          // Compute the ratio of the current transition stiffness to the original stiffness
          crack_flags_local(i) = (2.0 * a * etr + b) / Eo;
          cracking_locally_active = true;
        }
        else
        {
          crack_flags_local(i) = _crack_flags_old[_qp](i);
          cracking_locally_active = true;
        }
      }
    }

    if (cracking_locally_active)
    {
      // Update the elasticity tensor in the crack coordinate system
      const RealVectorValue & c = crack_flags_local;

      const bool c0_coupled = MooseUtils::absoluteFuzzyEqual(c(0), 1.0);
      const bool c1_coupled = MooseUtils::absoluteFuzzyEqual(c(1), 1.0);
      const bool c2_coupled = MooseUtils::absoluteFuzzyEqual(c(2), 1.0);

      const Real c01 = (c0_coupled && c1_coupled ? 1.0 : 0.0);
      const Real c02 = (c0_coupled && c2_coupled ? 1.0 : 0.0);
      const Real c12 = (c1_coupled && c2_coupled ? 1.0 : 0.0);

      const Real c01_shear_retention = (c0_coupled && c1_coupled ? 1.0 : _shear_retention_factor);
      const Real c02_shear_retention = (c0_coupled && c2_coupled ? 1.0 : _shear_retention_factor);
      const Real c12_shear_retention = (c1_coupled && c2_coupled ? 1.0 : _shear_retention_factor);

      // Filling with 9 components is sufficient because these are the only nonzero entries
      // for isotropic or orthotropic materials.
      std::vector<Real> local_elastic(9);

      local_elastic[0] = (c0_coupled ? _elasticity_tensor[_qp](0, 0, 0, 0) : c(0) * youngs_modulus);
      local_elastic[1] = _elasticity_tensor[_qp](0, 0, 1, 1) * c01;
      local_elastic[2] = _elasticity_tensor[_qp](0, 0, 2, 2) * c02;
      local_elastic[3] = (c1_coupled ? _elasticity_tensor[_qp](1, 1, 1, 1) : c(1) * youngs_modulus);
      local_elastic[4] = _elasticity_tensor[_qp](1, 1, 2, 2) * c12;
      local_elastic[5] = (c2_coupled ? _elasticity_tensor[_qp](2, 2, 2, 2) : c(2) * youngs_modulus);
      local_elastic[6] = _elasticity_tensor[_qp](1, 2, 1, 2) * c12_shear_retention;
      local_elastic[7] = _elasticity_tensor[_qp](0, 2, 0, 2) * c02_shear_retention;
      local_elastic[8] = _elasticity_tensor[_qp](0, 1, 0, 1) * c01_shear_retention;

      _local_elasticity_tensor.fillFromInputVector(local_elastic, RankFourTensor::symmetric9);

      // Rotate the modified elasticity tensor back into global coordinates
      _local_elasticity_tensor.rotate(R);
    }
  }
  if (!cracking_locally_active)
    _local_elasticity_tensor = _elasticity_tensor[_qp];
}

void
ComputeSmearedCrackingStress::crackingStressRotation()
{
  const Real youngs_modulus =
      ElasticityTensorTools::getIsotropicYoungsModulus(_elasticity_tensor[_qp]);
  const Real cracking_alpha = -youngs_modulus;

  Real cracking_stress = _cracking_stress[_qp];

  if (cracking_stress > 0)
  {
    // Initializing crack states
    _crack_rotation[_qp] = _crack_rotation_old[_qp];

    for (unsigned i = 0; i < 3; ++i)
    {
      _crack_max_strain[_qp](i) = _crack_max_strain_old[_qp](i);
      _crack_strain[_qp](i) = _crack_strain_old[_qp](i);
      _crack_flags[_qp](i) = _crack_flags_old[_qp](i);
    }

    // Compute crack orientations: updated _crack_rotation[_qp] based on current strain
    RealVectorValue principal_strain;
    computeCrackStrainAndOrientation(principal_strain);

    for (unsigned i = 0; i < 3; ++i)
    {
      if (principal_strain(i) > _crack_max_strain[_qp](i))
        _crack_max_strain[_qp](i) = principal_strain(i);
    }

    // Check for new cracks.
    // Rotate stress to cracked orientation.
    const RankTwoTensor & R = _crack_rotation[_qp];
    RankTwoTensor sigmaPrime(_stress[_qp]);
    sigmaPrime.rotate(R.transpose()); // stress in crack coordinates

    unsigned int num_cracks = 0;
    for (unsigned int i = 0; i < 3; ++i)
    {
      if (_crack_flags_old[_qp](i) < 1)
        ++num_cracks;
    }

    bool cracked(false);
    RealVectorValue sigma;
    for (unsigned int i = 0; i < 3; ++i)
    {
      sigma(i) = sigmaPrime(i, i);

      Real crackFactor(1);

      if (_cracking_release == CR_POWER)
        (*_crack_count)[_qp](i) = (*_crack_count_old)[_qp](i);

      if ((_cracking_release == CR_POWER && sigma(i) > cracking_stress &&
           _active_crack_planes[i] == 1))
      {
        cracked = true;
        ++((*_crack_count)[_qp](i));

        // Assume Poisson's ratio drops to zero for this direction.  Stiffness is then Young's
        // modulus.
        if ((*_crack_count_old)[_qp](i) == 0)
        {
          ++num_cracks;
          _crack_strain[_qp](i) = cracking_stress / youngs_modulus;
        }
        // Compute stress, factor....
        _crack_flags[_qp](i) *= 1. / 3.;

        if (_crack_max_strain[_qp](i) < _crack_strain[_qp](i))
          _crack_max_strain[_qp](i) = _crack_strain[_qp](i);

        sigma(i) = _crack_flags[_qp](i) * youngs_modulus * principal_strain(i);
      }
      else if (_cracking_release != CR_POWER && _crack_flags_old[_qp](i) == 1 &&
               sigma(i) > cracking_stress && num_cracks < _max_cracks &&
               _active_crack_planes[i] == 1)
      {
        // A new crack
        cracked = true;
        ++num_cracks;

        // Assume Poisson's ratio drops to zero for this direction.  Stiffness is then Young's
        // modulus.
        _crack_strain[_qp](i) = cracking_stress / youngs_modulus;

        if (_crack_max_strain[_qp](i) < _crack_strain[_qp](i))
          _crack_max_strain[_qp](i) = _crack_strain[_qp](i);

        crackFactor = computeCrackFactor(
            i, sigma(i), _crack_flags[_qp](i), cracking_stress, cracking_alpha, youngs_modulus);

        _crack_flags[_qp](i) = crackFactor;
      }

      else if (_cracking_release != CR_POWER && _crack_flags_old[_qp](i) < 1 &&
               std::abs(principal_strain(i) - _crack_max_strain[_qp](i)) < 1e-10)
      {
        // Previously cracked,
        // Crack opening
        cracked = true;
        crackFactor = computeCrackFactor(
            i, sigma(i), _crack_flags[_qp](i), cracking_stress, cracking_alpha, youngs_modulus);
        _crack_flags[_qp](i) = crackFactor;
      }

      else if (_cracking_neg_fraction > 0 &&
               _crack_strain[_qp](i) * _cracking_neg_fraction > principal_strain(i) &&
               -_crack_strain[_qp](i) * _cracking_neg_fraction < principal_strain(i))
      {
        cracked = true;
        const Real etr = _cracking_neg_fraction * _crack_strain[_qp](i);
        const Real Eo = cracking_stress / _crack_strain[_qp](i);
        const Real Ec = Eo * _crack_flags_old[_qp](i);
        const Real a = (Ec - Eo) / (4 * etr);
        const Real b = (Ec + Eo) / 2;
        const Real c = (Ec - Eo) * etr / 4;
        sigma(i) = (a * principal_strain(i) + b) * principal_strain(i) + c;
      }
    }

    if (cracked)
      applyCracksToTensor(_stress[_qp], sigma);
  }
}

void
ComputeSmearedCrackingStress::computeCrackStrainAndOrientation(RealVectorValue & principal_strain)
{
  // The rotation tensor is ordered such that known dirs appear last in the list of
  // columns.  So, if one dir is known, it corresponds with the last column in the
  // rotation tensor.
  //
  // This convention is based on the eigen routine returning eigen values in
  // ascending order.
  const unsigned int numKnownDirs = getNumKnownCrackDirs();

  if (numKnownDirs == 0)
  {
    std::vector<Real> eigval(3, 0.0);
    RankTwoTensor eigvec;

    _elastic_strain[_qp].symmetricEigenvaluesEigenvectors(eigval, eigvec);

    // If the elastic strain is beyond the cracking strain, save the eigen vectors as
    // the rotation tensor.
    _crack_rotation[_qp] = eigvec;

    principal_strain(0) = eigval[0];
    principal_strain(1) = eigval[1];
    principal_strain(2) = eigval[2];
  }
  else if (numKnownDirs == 1)
  {
    // This is easily the most complicated case.
    // 1.  Rotate the elastic strain to the orientation associated with the known
    //     crack.
    // 2.  Extract the upper 2x2 diagonal block into a separate tensor.
    // 3.  Run the eigen solver on the result.
    // 4.  Update the rotation tensor to reflect the effect of the 2 eigenvectors.

    // 1.
    const RankTwoTensor & R = _crack_rotation[_qp];
    RankTwoTensor ePrime(_elastic_strain[_qp]);
    ePrime.rotate(R.transpose()); // elastic strain in principal coordinates

    // 2.
    ColumnMajorMatrix e2x2(2, 2);
    e2x2(0, 0) = ePrime(0, 0);
    e2x2(1, 0) = ePrime(1, 0);
    e2x2(0, 1) = ePrime(0, 1);
    e2x2(1, 1) = ePrime(1, 1);

    // 3.
    ColumnMajorMatrix e_val2x1(2, 1);
    ColumnMajorMatrix e_vec2x2(2, 2);
    e2x2.eigen(e_val2x1, e_vec2x2);

    // 4.
    ColumnMajorMatrix e_vec(3, 3);
    e_vec(0, 0) = e_vec2x2(0, 0);
    e_vec(1, 0) = e_vec2x2(1, 0);
    e_vec(2, 0) = 0;
    e_vec(0, 1) = e_vec2x2(0, 1);
    e_vec(1, 1) = e_vec2x2(1, 1);
    e_vec(2, 1) = 0;
    e_vec(2, 0) = 0;
    e_vec(2, 1) = 0;
    e_vec(2, 2) = 1;

    RankTwoTensor eigvec(e_vec(0, 0),
                         e_vec(1, 0),
                         e_vec(2, 0),
                         e_vec(0, 1),
                         e_vec(1, 1),
                         e_vec(2, 1),
                         e_vec(0, 2),
                         e_vec(1, 2),
                         e_vec(2, 2));

    _crack_rotation[_qp] = _crack_rotation_old[_qp] * eigvec; // Roe implementation

    principal_strain(0) = e_val2x1(0, 0);
    principal_strain(1) = e_val2x1(1, 0);
    principal_strain(2) = ePrime(2, 2);
  }
  else if (numKnownDirs == 2 || numKnownDirs == 3)
  {
    // Rotate to cracked orientation and pick off the strains in the rotated
    // coordinate directions.
    const RankTwoTensor & R = _crack_rotation[_qp];
    RankTwoTensor ePrime(_elastic_strain[_qp]);
    ePrime.rotate(R.transpose()); // elastic strain in principal coordinates

    principal_strain(0) = ePrime(0, 0);
    principal_strain(1) = ePrime(1, 1);
    principal_strain(2) = ePrime(2, 2);
  }
  else
    mooseError("Invalid number of known crack directions");
}

unsigned int
ComputeSmearedCrackingStress::getNumKnownCrackDirs() const
{
  const unsigned fromElement = 0;
  unsigned int retVal = 0;
  for (unsigned int i = 0; i < 3 - fromElement; ++i)
    retVal += (_crack_flags_old[_qp](i) < 1);
  return retVal + fromElement;
}

Real
ComputeSmearedCrackingStress::computeCrackFactor(int i,
                                                 Real & sigma,
                                                 Real & flag_value,
                                                 const Real cracking_stress,
                                                 const Real cracking_alpha,
                                                 const Real youngs_modulus)
{
  if (_cracking_release == CR_EXPONENTIAL)
  {
    if (_crack_max_strain[_qp](i) < _crack_strain[_qp](i))
    {
      mooseError("Max strain less than crack strain: ",
                 i,
                 " ",
                 sigma,
                 ", ",
                 _crack_max_strain[_qp](i),
                 ", ",
                 _crack_strain[_qp](i),
                 ", ",
                 _elastic_strain[_qp](0, 0),
                 ", ",
                 _elastic_strain[_qp](1, 1),
                 ", ",
                 _elastic_strain[_qp](2, 2),
                 ", ",
                 _elastic_strain[_qp](0, 1),
                 ", ",
                 _elastic_strain[_qp](0, 2),
                 ", ",
                 _elastic_strain[_qp](1, 2));
    }
    const Real crackMaxStrain = _crack_max_strain[_qp](i);
    // Compute stress that follows exponental curve
    sigma = cracking_stress * (_cracking_residual_stress +
                               (1.0 - _cracking_residual_stress) *
                                   std::exp(cracking_alpha * _cracking_beta / cracking_stress *
                                            (crackMaxStrain - _crack_strain[_qp](i))));
    // Compute ratio of current stiffness to original stiffness
    flag_value = sigma * _crack_strain[_qp](i) / (crackMaxStrain * cracking_stress);
  }
  else
  {
    if (_cracking_residual_stress == 0)
    {
      const Real tiny = 1e-16;
      flag_value = tiny;
      sigma = tiny * _crack_strain[_qp](i) * youngs_modulus;
    }
    else
    {
      sigma = _cracking_residual_stress * cracking_stress;
      flag_value = sigma / (_crack_max_strain[_qp](i) * youngs_modulus);
    }
  }
  if (flag_value < 0)
  {
    std::stringstream err;
    err << "Negative crack flag found: " << i << " " << flag_value << ", "
        << _crack_max_strain[_qp](i) << ", " << _crack_strain[_qp](i) << ", " << std::endl;
    mooseError(err.str());
  }
  return flag_value;
}

void
ComputeSmearedCrackingStress::applyCracksToTensor(RankTwoTensor & tensor,
                                                  const RealVectorValue & sigma)
{
  // Get transformation matrix
  const RankTwoTensor & R = _crack_rotation[_qp];
  // Rotate to crack frame
  tensor.rotate(R.transpose());

  // Reset stress if cracked
  for (unsigned int i = 0; i < 3; ++i)
    if (_crack_flags[_qp](i) < 1)
    {
      const Real stress_correction_ratio = (tensor(i, i) - sigma(i)) / tensor(i, i);
      if (stress_correction_ratio > _max_stress_correction)
        tensor(i, i) *= (1.0 - _max_stress_correction);
      else if (stress_correction_ratio < -_max_stress_correction)
        tensor(i, i) *= (1.0 + _max_stress_correction);
      else
        tensor(i, i) = sigma(i);
    }

  // Rotate back to global frame
  tensor.rotate(R);
}

bool
ComputeSmearedCrackingStress::previouslyCracked()
{
  for (unsigned int i = 0; i < 3; ++i)
    if (_crack_flags_old[_qp](i) < 1.0)
      return true;
  return false;
}
