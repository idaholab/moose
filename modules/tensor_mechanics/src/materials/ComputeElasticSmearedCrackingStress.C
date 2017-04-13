/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeElasticSmearedCrackingStress.h"

template <>
InputParameters
validParams<ComputeElasticSmearedCrackingStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Compute stress using elasticity for finite strains");
  params.addParam<std::string>(
      "cracking_release",
      "abrupt",
      "The cracking release type.  Choices are abrupt (default) and exponential.");
  params.addParam<Real>(
      "cracking_residual_stress",
      0.0,
      "The fraction of the cracking stress allowed to be maintained following a crack.");
  params.addRequiredParam<FunctionName>(
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
  return params;
}

namespace
{
ComputeElasticSmearedCrackingStress::CRACKING_RELEASE
getCrackingModel(const std::string & name)
{
  std::string n(name);
  std::transform(n.begin(), n.end(), n.begin(), ::tolower);
  ComputeElasticSmearedCrackingStress::CRACKING_RELEASE cm(
      ComputeElasticSmearedCrackingStress::CR_UNKNOWN);
  if (n == "abrupt")
    cm = ComputeElasticSmearedCrackingStress::CR_ABRUPT;
  else if (n == "exponential")
    cm = ComputeElasticSmearedCrackingStress::CR_EXPONENTIAL;
  else if (n == "power")
    cm = ComputeElasticSmearedCrackingStress::CR_POWER;
  if (cm == ComputeElasticSmearedCrackingStress::CR_UNKNOWN)
    mooseError("Unknown cracking model");
  return cm;
}
}

ComputeElasticSmearedCrackingStress::ComputeElasticSmearedCrackingStress(
    const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _mechanical_strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "mechanical_strain")),
    _strain_increment(getDefaultMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(
        getDefaultMaterialProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _stress_old(declarePropertyOld<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "elastic_strain")),
    _cracking_release(getCrackingModel(getParam<std::string>("cracking_release"))),
    _cracking_residual_stress(getParam<Real>("cracking_residual_stress")),
    _cracking_stress_function(getFunction("cracking_stress")),
    _cracking_alpha(0),
    _active_crack_planes(3, 1),
    _max_cracks(getParam<unsigned int>("max_cracks")),
    _cracking_neg_fraction(getParam<Real>("cracking_neg_fraction")),
    _crack_flags(NULL),
    _crack_flags_old(NULL),
    _crack_count(NULL),
    _crack_count_old(NULL),
    _crack_rotation(NULL),
    _crack_rotation_old(NULL),
    _crack_strain(NULL),
    _crack_strain_old(NULL),
    _crack_max_strain(NULL),
    _crack_max_strain_old(NULL),
    _principal_strain(3, 1)
{
  _crack_flags = &declareProperty<RealVectorValue>(_base_name + "crack_flags");
  _crack_flags_old = &declarePropertyOld<RealVectorValue>(_base_name + "crack_flags");
  if (_cracking_release == CR_POWER)
  {
    _crack_count = &declareProperty<RealVectorValue>(_base_name + "crack_count");
    _crack_count_old = &declarePropertyOld<RealVectorValue>(_base_name + "crack_count");
  }
  _crack_rotation = &declareProperty<RankTwoTensor>(_base_name + "crack_rotation");
  _crack_rotation_old = &declarePropertyOld<RankTwoTensor>(_base_name + "crack_rotation");
  _crack_max_strain = &declareProperty<RealVectorValue>(_base_name + "crack_max_strain");
  _crack_max_strain_old = &declarePropertyOld<RealVectorValue>(_base_name + "crack_max_strain");
  _crack_strain = &declareProperty<RealVectorValue>(_base_name + "crack_strain");
  _crack_strain_old = &declarePropertyOld<RealVectorValue>(_base_name + "crack_strain");

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
ComputeElasticSmearedCrackingStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();

  _stress_old[_qp] = _stress[_qp];
  _elastic_strain_old[_qp] = _elastic_strain[_qp];

  (*_crack_flags)[_qp](0) = (*_crack_flags)[_qp](1) = (*_crack_flags)[_qp](2) =
      (*_crack_flags_old)[_qp](0) = (*_crack_flags_old)[_qp](1) = (*_crack_flags_old)[_qp](2) = 1;

  if (_crack_count)
  {
    (*_crack_count)[_qp] = 0;
    (*_crack_count_old)[_qp] = 0;
  }

  (*_crack_strain)[_qp] = 0;
  (*_crack_strain_old)[_qp] = 0;

  (*_crack_max_strain)[_qp](0) = 0;
  (*_crack_max_strain_old)[_qp] = 0;

  (*_crack_rotation)[_qp].Identity();
  (*_crack_rotation_old)[_qp].Identity();
}

void
ComputeElasticSmearedCrackingStress::initialSetup()
{
  _is_finite_strain = hasBlockMaterialProperty<RankTwoTensor>(_base_name + "strain_increment");
}

void
ComputeElasticSmearedCrackingStress::computeQpProperties()
{
  computeQpStress();

  _Jacobian_mult[_qp] = _local_elasticity_tensor; // This is NOT the exact jacobian

  // Add in extra stress
  _stress[_qp] += _extra_stress[_qp];
}

void
ComputeElasticSmearedCrackingStress::computeQpStress()
{
  _elastic_strain[_qp] = _mechanical_strain[_qp];

  // update elasticity tensor with old cracking status: crack_flags_old and crack_orientation_old
  updateElasticityTensor();

  // calculate trial stress
  if (_is_finite_strain)
  {
    // Calculate stress in intermediate configuration
    RankTwoTensor intermediate_stress =
        _stress_old[_qp] + _local_elasticity_tensor * _strain_increment[_qp];

    // Rotate the stress to the current configuration
    _stress[_qp] =
        _rotation_increment[_qp] * intermediate_stress * _rotation_increment[_qp].transpose();
  }
  else
    _stress[_qp] = _local_elasticity_tensor * _elastic_strain[_qp];

  // compute crack status and adjust stress
  crackingStressRotation();
}

void
ComputeElasticSmearedCrackingStress::updateElasticityTensor()
{
  _local_elasticity_tensor = _elasticity_tensor[_qp];

  // assuming isotropic elasticity tensorl at the begining
  Real lambda = _local_elasticity_tensor(0, 0, 1, 1);
  Real mu = _local_elasticity_tensor(0, 1, 0, 1);

  _youngs_modulus = mu * (3 * lambda + 2 * mu) / (lambda + mu);

  bool cracking_locally_active = false;

  Real cracking_stress = _cracking_stress_function.value(_t, _q_point[_qp]);

  if (cracking_stress > 0)
  {
    RankTwoTensor RT((*_crack_rotation_old)[_qp].transpose());
    RankTwoTensor ePrime = RT * _elastic_strain[_qp] * RT.transpose();

    for (unsigned int i = 0; i < 3; ++i)
    {
      // new implementation: update elasticity tensor based on crack status of the end of last time
      // step
      _crack_flags_local(i) = 1;

      if ((*_crack_flags_old)[_qp](i) < 1.0)
      {
        if (_cracking_neg_fraction == 0 && ePrime(i, i) < 0)
          _crack_flags_local(i) = 1;
        else if (_cracking_neg_fraction > 0 &&
                 ePrime(i, i) < (*_crack_strain)[_qp](i) * _cracking_neg_fraction &&
                 ePrime(i, i) > -(*_crack_strain)[_qp](i) * _cracking_neg_fraction)
        {
          const Real etr = _cracking_neg_fraction * (*_crack_strain)[_qp](i);
          const Real Eo = cracking_stress / (*_crack_strain)[_qp](i);
          const Real Ec = Eo * (*_crack_flags_old)[_qp](i);
          const Real a = (Ec - Eo) / (4 * etr);
          const Real b = (Ec + Eo) / 2;
          // Compute the ratio of the current transition stiffness to the original stiffness
          _crack_flags_local(i) = (2 * a * etr + b) / Eo;
          cracking_locally_active = true;
        }
        else
        {
          _crack_flags_local(i) = (*_crack_flags_old)[_qp](i);
          cracking_locally_active = true;
        }
      }
    }

    if (cracking_locally_active)
    {
      // Adjust the elasticity matrix for cracking.  This must be used by the constitutive law.
      RealTensorValue Q;
      Q(0, 0) = RT(0, 0);
      Q(0, 1) = RT(0, 1);
      Q(0, 2) = RT(0, 2);

      Q(1, 0) = RT(1, 0);
      Q(1, 1) = RT(1, 1);
      Q(1, 2) = RT(1, 2);

      Q(2, 0) = RT(2, 0);
      Q(2, 1) = RT(2, 1);
      Q(2, 2) = RT(2, 2);

      // First rotate the elasticity tensor into crack frame coordinate
      _local_elasticity_tensor.rotate(Q);

      // Then update the elasticity tensor in crack frame
      const RealVectorValue & c = _crack_flags_local;
      const Real c0_coupled = (c(0) < 1 ? 0 : 1);
      const Real c1_coupled = (c(1) < 1 ? 0 : 1);
      const Real c2_coupled = (c(2) < 1 ? 0 : 1);

      const Real c01 = c0_coupled * c1_coupled;
      const Real c02 = c0_coupled * c2_coupled;
      const Real c12 = c1_coupled * c2_coupled;
      const Real c012 = c0_coupled * c12;

      const Real ym = _youngs_modulus;
      Real val[3];
      val[0] = _local_elasticity_tensor(0, 0, 0, 0);
      val[1] = _local_elasticity_tensor(1, 1, 1, 1);
      val[2] = _local_elasticity_tensor(2, 2, 2, 2);

      // Assume Poisson's ratio goes to zero for the cracked direction.
      _local_elasticity_tensor(0, 0, 0, 0) = (c(0) < 1 ? c(0) * ym : val[0]);
      _local_elasticity_tensor(0, 0, 1, 1) *= c01;
      _local_elasticity_tensor(0, 0, 2, 2) *= c02;
      _local_elasticity_tensor(0, 0, 1, 2) *= c01;
      _local_elasticity_tensor(0, 0, 0, 2) *= c012;
      _local_elasticity_tensor(0, 0, 0, 1) *= c02;

      _local_elasticity_tensor(1, 1, 1, 1) = (c(1) < 1 ? c(1) * ym : val[1]);
      _local_elasticity_tensor(1, 1, 2, 2) *= c12;
      _local_elasticity_tensor(1, 1, 1, 2) *= c01;
      _local_elasticity_tensor(0, 2, 1, 1) *= c12;
      _local_elasticity_tensor(0, 1, 1, 1) *= c012;

      _local_elasticity_tensor(2, 2, 2, 2) = (c(2) < 1 ? c(2) * ym : val[2]);
      _local_elasticity_tensor(1, 2, 2, 2) *= c012;
      _local_elasticity_tensor(0, 2, 2, 2) *= c12;
      _local_elasticity_tensor(0, 1, 2, 2) *= c02;

      _local_elasticity_tensor(1, 2, 1, 2) *= c01;
      _local_elasticity_tensor(0, 2, 1, 2) *= c012;
      _local_elasticity_tensor(0, 1, 1, 2) *= c012;

      _local_elasticity_tensor(0, 2, 0, 2) *= c12;
      _local_elasticity_tensor(0, 1, 0, 2) *= c012;

      _local_elasticity_tensor(0, 1, 0, 1) *= c02;

      // fill in from symmetry relations
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int j = 0; j < 3; ++j)
          for (unsigned int k = 0; k < 3; ++k)
            for (unsigned int l = 0; l < 3; ++l)
              _local_elasticity_tensor(i, j, l, k) = _local_elasticity_tensor(j, i, k, l) =
                  _local_elasticity_tensor(j, i, l, k) = _local_elasticity_tensor(k, l, i, j) =
                      _local_elasticity_tensor(l, k, j, i) = _local_elasticity_tensor(k, l, j, i) =
                          _local_elasticity_tensor(l, k, i, j) =
                              _local_elasticity_tensor(i, j, k, l);

      // rotate the elasticity tensor back into global coordinates
      RealTensorValue QT(Q.transpose());

      _local_elasticity_tensor.rotate(QT);
    }
  }
}

void
ComputeElasticSmearedCrackingStress::crackingStressRotation()
{
  Real lambda = _elasticity_tensor[_qp](0, 0, 1, 1);
  Real mu = _elasticity_tensor[_qp](0, 1, 0, 1);
  _youngs_modulus = mu * (3 * lambda + 2 * mu) / (lambda + mu);
  _cracking_alpha = -_youngs_modulus;

  Real cracking_stress = _cracking_stress_function.value(_t, _q_point[_qp]);

  if (cracking_stress > 0)
  {
    // Initializing crack states
    (*_crack_rotation)[_qp] = (*_crack_rotation_old)[_qp];

    for (unsigned i = 0; i < 3; ++i)
    {
      (*_crack_max_strain)[_qp](i) = (*_crack_max_strain_old)[_qp](i);
      (*_crack_strain)[_qp](i) = (*_crack_strain_old)[_qp](i);
      (*_crack_flags)[_qp](i) = (*_crack_flags_old)[_qp](i);
    }

    // Compute crack orientations: updated (*_crack_rotation)[_qp] based on current sstrain
    computeCrackStrainAndOrientation(_principal_strain);

    for (unsigned i = 0; i < 3; ++i)
    {
      if (_principal_strain(i, 0) > (*_crack_max_strain)[_qp](i))
        (*_crack_max_strain)[_qp](i) = _principal_strain(i, 0);
    }

    // Check for new cracks.
    // Rotate stress to cracked orientation.
    RankTwoTensor RT((*_crack_rotation)[_qp].transpose());
    RankTwoTensor sigmaPrime = RT * _stress[_qp] * RT.transpose(); // stress in principal
                                                                   // coordinates

    unsigned int num_cracks = 0;
    for (unsigned int i = 0; i < 3; ++i)
    {
      _crack_flags_local(i) = 1;
      if ((*_crack_flags_old)[_qp](i) < 1)
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
        const Real stiff = _youngs_modulus;

        if ((*_crack_count_old)[_qp](i) == 0)
        {
          ++num_cracks;
          (*_crack_strain)[_qp](i) = cracking_stress / stiff;
        }
        // Compute stress, factor....
        (*_crack_flags)[_qp](i) *= 1. / 3.;

        if ((*_crack_max_strain)[_qp](i) < (*_crack_strain)[_qp](i))
          (*_crack_max_strain)[_qp](i) = (*_crack_strain)[_qp](i);

        sigma(i) = (*_crack_flags)[_qp](i) * stiff * _principal_strain(i, 0);
      }
      else if (_cracking_release != CR_POWER && (*_crack_flags_old)[_qp](i) == 1 &&
               sigma(i) > cracking_stress && num_cracks < _max_cracks &&
               _active_crack_planes[i] == 1)
      {
        // A new crack
        cracked = true;
        ++num_cracks;

        // Assume Poisson's ratio drops to zero for this direction.  Stiffness is then Young's
        // modulus.
        const Real stiff = _youngs_modulus;

        (*_crack_strain)[_qp](i) = cracking_stress / stiff;

        if ((*_crack_max_strain)[_qp](i) < (*_crack_strain)[_qp](i))
          (*_crack_max_strain)[_qp](i) = (*_crack_strain)[_qp](i);

        crackFactor = computeCrackFactor(i, sigma(i), (*_crack_flags)[_qp](i), cracking_stress);

        (*_crack_flags)[_qp](i) = crackFactor;
        _crack_flags_local(i) = crackFactor;
      }

      else if (_cracking_release != CR_POWER && (*_crack_flags_old)[_qp](i) < 1 &&
               std::abs(_principal_strain(i, 0) - (*_crack_max_strain)[_qp](i)) < 1e-10)
      {
        // Previously cracked,
        // Crack opening
        cracked = true;
        crackFactor = computeCrackFactor(i, sigma(i), (*_crack_flags)[_qp](i), cracking_stress);
        (*_crack_flags)[_qp](i) = crackFactor;
        _crack_flags_local(i) = crackFactor;
      }

      else if (_cracking_neg_fraction > 0 &&
               (*_crack_strain)[_qp](i) * _cracking_neg_fraction > _principal_strain(i, 0) &&
               -(*_crack_strain)[_qp](i) * _cracking_neg_fraction < _principal_strain(i, 0))
      {
        cracked = true;
        const Real etr = _cracking_neg_fraction * (*_crack_strain)[_qp](i);
        const Real Eo = cracking_stress / (*_crack_strain)[_qp](i);
        const Real Ec = Eo * (*_crack_flags_old)[_qp](i);
        const Real a = (Ec - Eo) / (4 * etr);
        const Real b = (Ec + Eo) / 2;
        const Real c = (Ec - Eo) * etr / 4;
        sigma(i) = (a * _principal_strain(i, 0) + b) * _principal_strain(i, 0) + c;
      }
    }

    if (cracked)
      applyCracksToTensor(_stress[_qp], sigma);
  }
}

void
ComputeElasticSmearedCrackingStress::computeCrackStrainAndOrientation(
    ColumnMajorMatrix & principal_strain)
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
    (*_crack_rotation)[_qp] = eigvec;

    principal_strain(0, 0) = eigval[0];
    principal_strain(1, 0) = eigval[1];
    principal_strain(2, 0) = eigval[2];
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
    RankTwoTensor R((*_crack_rotation)[_qp]);
    RankTwoTensor ePrime =
        R.transpose() * _elastic_strain[_qp] * R; // elastic_strain tensor in principial coordinate

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

    (*_crack_rotation)[_qp] = (*_crack_rotation_old)[_qp] * eigvec; // Roe implementation

    principal_strain(0, 0) = e_val2x1(0, 0);
    principal_strain(1, 0) = e_val2x1(1, 0);
    principal_strain(2, 0) = ePrime(2, 2);
  }
  else if (numKnownDirs == 2 || numKnownDirs == 3)
  {
    // Rotate to cracked orientation and pick off the strains in the rotated
    // coordinate directions.
    RankTwoTensor R((*_crack_rotation)[_qp]);
    RankTwoTensor ePrime = R.transpose() * _elastic_strain[_qp] * R;

    principal_strain(0, 0) = ePrime(0, 0);
    principal_strain(1, 0) = ePrime(1, 1);
    principal_strain(2, 0) = ePrime(2, 2);
  }
  else
    mooseError("Invalid number of known crack directions");
}

unsigned int
ComputeElasticSmearedCrackingStress::getNumKnownCrackDirs() const
{
  const unsigned fromElement = 0;
  unsigned int retVal = 0;
  for (unsigned int i = 0; i < 3 - fromElement; ++i)
    retVal += ((*_crack_flags_old)[_qp](i) < 1);
  return retVal + fromElement;
}

Real
ComputeElasticSmearedCrackingStress::computeCrackFactor(int i,
                                                        Real & sigma,
                                                        Real & flag_value,
                                                        const Real & cracking_stress)
{
  if (_cracking_release == CR_EXPONENTIAL)
  {
    if ((*_crack_max_strain)[_qp](i) < (*_crack_strain)[_qp](i))
    {
      mooseError("Max strain less than crack strain: ",
                 i,
                 " ",
                 sigma,
                 ", ",
                 (*_crack_max_strain)[_qp](i),
                 ", ",
                 (*_crack_strain)[_qp](i),
                 ", ",
                 _principal_strain(0, 0),
                 ", ",
                 _principal_strain(1, 0),
                 ", ",
                 _principal_strain(2, 0),
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
    const Real crackMaxStrain = (*_crack_max_strain)[_qp](i);
    // Compute stress that follows exponental curve
    sigma = cracking_stress * (_cracking_residual_stress +
                               (1.0 - _cracking_residual_stress) *
                                   std::exp(_cracking_alpha / cracking_stress *
                                            (crackMaxStrain - (*_crack_strain)[_qp](i))));
    // Compute ratio of current stiffness to original stiffness
    flag_value = sigma * (*_crack_strain)[_qp](i) / (crackMaxStrain * cracking_stress);
  }
  else
  {
    if (_cracking_residual_stress == 0)
    {
      const Real tiny = 1e-16;
      flag_value = tiny;
      sigma = tiny * (*_crack_strain)[_qp](i) * _youngs_modulus;
    }
    else
    {
      sigma = _cracking_residual_stress * cracking_stress;
      flag_value = sigma / ((*_crack_max_strain)[_qp](i) * _youngs_modulus);
    }
  }
  if (flag_value < 0)
  {
    std::stringstream err;
    err << "Negative crack flag found: " << i << " " << flag_value << ", "
        << (*_crack_max_strain)[_qp](i) << ", " << (*_crack_strain)[_qp](i) << ", " << std::endl;
    mooseError(err.str());
  }
  return flag_value;
}

void
ComputeElasticSmearedCrackingStress::applyCracksToTensor(RankTwoTensor & tensor,
                                                         const RealVectorValue & sigma)
{
  // Form transformation matrix R*E*R^T
  const RankTwoTensor & R((*_crack_rotation)[_qp]);
  // Rotate to crack frame
  RankTwoTensor temp_tensor = R.transpose() * tensor * R;

  tensor = temp_tensor;

  // Reset stress if cracked
  for (unsigned int i = 0; i < 3; ++i)
    if ((*_crack_flags)[_qp](i) < 1)
      tensor(i, i) = sigma(i);

  // Rotate back to global frame
  temp_tensor = R * tensor * R.transpose();
  tensor = temp_tensor;
}
