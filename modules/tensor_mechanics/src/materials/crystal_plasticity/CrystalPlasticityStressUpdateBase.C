//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityStressUpdateBase.h"

#include "libmesh/utility.h"
#include "Conversion.h"
#include "MooseException.h"

InputParameters
CrystalPlasticityStressUpdateBase::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>(
      "base_name",
      "Optional parameter that allows the user to define multiple crystal plasticity mechanisms");
  params.addClassDescription(
      "Crystal Plasticity base class: handles the Newton iteration over the stress residual and "
      "calculates the Jacobian based on constitutive laws provided by inheriting classes");

  // The return stress increment classes are intended to be iterative materials, so must set compute
  // = false for all inheriting classes
  params.set<bool>("compute") = false;
  params.suppressParameter<bool>("compute");

  params.addRequiredParam<unsigned int>(
      "number_slip_systems",
      "The total number of possible active slip systems for the crystalline material");
  params.addRequiredParam<FileName>("slip_sys_file_name",
                                    "Name of the file containing the slip systems");
  params.addParam<Real>("number_cross_slip_directions",
                        0,
                        "Quanity of unique slip directions, used to determine cross slip familes");
  params.addParam<Real>("number_cross_slip_planes",
                        0,
                        "Quanity of slip planes belonging to a single cross slip direction; used "
                        "to determine cross slip families");
  params.addParam<Real>(
      "slip_increment_tolerance",
      2e-2,
      "Maximum allowable slip in an increment for each individual constitutive model");
  params.addParam<Real>(
      "stol", 1e-2, "Constitutive internal state variable relative change tolerance");
  params.addParam<Real>("resistance_tol",
                        1.0e-2,
                        "Constitutive slip system resistance relative residual tolerance for each "
                        "individual constitutive model");
  params.addParam<Real>("zero_tol",
                        1e-12,
                        "Tolerance for residual check when variable value is zero for each "
                        "individual constitutive model");
  return params;
}

CrystalPlasticityStressUpdateBase::CrystalPlasticityStressUpdateBase(
    const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _number_slip_systems(getParam<unsigned int>("number_slip_systems")),
    _slip_sys_file_name(getParam<FileName>("slip_sys_file_name")),
    _number_cross_slip_directions(getParam<Real>("number_cross_slip_directions")),
    _number_cross_slip_planes(getParam<Real>("number_cross_slip_planes")),

    _rel_state_var_tol(getParam<Real>("stol")),
    _slip_incr_tol(getParam<Real>("slip_increment_tolerance")),
    _resistance_tol(getParam<Real>("resistance_tol")),
    _zero_tol(getParam<Real>("zero_tol")),

    _slip_resistance(declareProperty<std::vector<Real>>(_base_name + "slip_resistance")),
    _slip_resistance_old(getMaterialPropertyOld<std::vector<Real>>(_base_name + "slip_resistance")),
    _slip_increment(declareProperty<std::vector<Real>>(_base_name + "slip_increment")),

    _slip_direction(_number_slip_systems * LIBMESH_DIM),
    _slip_plane_normal(_number_slip_systems * LIBMESH_DIM),
    _flow_direction(declareProperty<std::vector<RankTwoTensor>>(_base_name + "flow_direction")),
    _tau(declareProperty<std::vector<Real>>(_base_name + "applied_shear_stress"))
{
  _substep_dt = 0.0;

  getSlipSystems();
  sortCrossSlipFamilies();

  if (parameters.isParamSetByUser("number_cross_slip_directions"))
    _calculate_cross_slip = true;
  else
    _calculate_cross_slip = false;
}

void
CrystalPlasticityStressUpdateBase::initQpStatefulProperties()
{
  _tau[_qp].resize(_number_slip_systems);

  _flow_direction[_qp].resize(_number_slip_systems);
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
  {
    _flow_direction[_qp][i].zero();
    _tau[_qp][i] = 0.0;
  }

  _slip_resistance[_qp].resize(_number_slip_systems);
  _slip_increment[_qp].resize(_number_slip_systems);
}

void
CrystalPlasticityStressUpdateBase::getSlipSystems()
{
  bool orthonormal_error = false;

  getPlaneNormalAndDirectionVectors(_slip_sys_file_name,
                                    _number_slip_systems,
                                    _slip_plane_normal,
                                    _slip_direction,
                                    orthonormal_error);

  if (orthonormal_error)
    mooseError("CrystalPlasticityStressUpdateBase Error: The slip system file contains a slip "
               "direction and "
               "plane normal pair that are not orthonormal");
}

void
CrystalPlasticityStressUpdateBase::getPlaneNormalAndDirectionVectors(
    const FileName & vector_file_name,
    const unsigned int & number_dislocation_systems,
    DenseVector<Real> & plane_normal_vector,
    DenseVector<Real> & direction_vector,
    bool & orthonormal_error)
{
  mooseAssert(
      LIBMESH_DIM == 3,
      "Crystal plasticity slip plane normal and slip direction needs to have sizes equal to 3");

  Real vec[LIBMESH_DIM];
  std::ifstream fileslipsys;

  MooseUtils::checkFileReadable(vector_file_name);

  fileslipsys.open(vector_file_name.c_str());

  for (unsigned int i = 0; i < number_dislocation_systems; ++i)
  {
    // Read the plane normal
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      if (!(fileslipsys >> vec[j]))
        mooseError(
            "CrystalPlasticityStressUpdateBase Error: Premature end of file reading plane normal "
            "vectors from the file ",
            vector_file_name);

    // Normalize the vectors
    Real magnitude = 0.0;
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      magnitude += Utility::pow<2>(vec[j]);
    magnitude = std::sqrt(magnitude);

    for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      plane_normal_vector(i * LIBMESH_DIM + j) = vec[j] / magnitude;

    // Read the direction
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      if (!(fileslipsys >> vec[j]))
        mooseError("CrystalPlasticityStressUpdateBase Error: Premature end of file reading "
                   "direction vectors "
                   "from the file ",
                   vector_file_name);

    // Normalize the vectors
    magnitude = Utility::pow<2>(vec[0]) + Utility::pow<2>(vec[1]) + Utility::pow<2>(vec[2]);
    magnitude = std::sqrt(magnitude);

    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      direction_vector(i * LIBMESH_DIM + j) = vec[j] / magnitude;

    // Check that the normalized vectors are orthonormal
    magnitude = 0.0;
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      magnitude += direction_vector(i * LIBMESH_DIM + j) * plane_normal_vector(i * LIBMESH_DIM + j);

    if (std::abs(magnitude) > libMesh::TOLERANCE)
    {
      orthonormal_error = true;
      break;
    }
    if (orthonormal_error)
      break;
  }

  fileslipsys.close();
}

void
CrystalPlasticityStressUpdateBase::sortCrossSlipFamilies()
{
  if (_number_cross_slip_directions == 0)
  {
    _cross_slip_familes.resize(0);
    return;
  }

  // If cross slip does occur, then set up the system of vectors for the families
  _cross_slip_familes.resize(_number_cross_slip_directions);
  // and set the first index of each inner vector
  for (unsigned int i = 0; i < _number_cross_slip_directions; ++i)
    _cross_slip_familes[i].resize(1);

  // Sort the index of the slip system based vectors into separte families
  unsigned int family_counter = 1;
  _cross_slip_familes[0][0] = 0;

  for (unsigned int i = 1; i < _number_slip_systems; ++i)
  {
    for (unsigned int j = 0; j < family_counter; ++j)
    {
      // check to see if the slip system direction i matches any of the existing slip directions
      // First calculate the dot product
      Real dot_product = 0.0;
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        unsigned int check_family_index = _cross_slip_familes[j][0];
        dot_product += std::abs(_slip_direction(check_family_index * LIBMESH_DIM + k) -
                                _slip_direction(i * LIBMESH_DIM + k));
      }
      // Then check if the dot product is one, if yes, add to family and break
      if (MooseUtils::absoluteFuzzyEqual(dot_product, 0.0))
      {
        _cross_slip_familes[j].push_back(i);
        if (_cross_slip_familes[j].size() > _number_cross_slip_planes)
          mooseError(
              "Exceeded the number of cross slip planes allowed in a single cross slip family");

        break; // exit the loop over the exisiting cross slip families and move to the next slip
               // direction
      }
      // The slip system in question does not belong to an existing family
      else if (j == (family_counter - 1) && !MooseUtils::absoluteFuzzyEqual(dot_product, 0.0))
      {
        if (family_counter > _number_cross_slip_directions)
          mooseError("Exceeds the number of cross slip directions specified for this material");

        _cross_slip_familes[family_counter][0] = i;
        family_counter++;
        break;
      }
    }
  }

#ifdef DEBUG
  mooseWarning("Checking the slip system ordering now:");
  for (unsigned int i = 0; i < _number_cross_slip_directions; ++i)
  {
    Moose::out << "In cross slip family " << i << std::endl;
    for (unsigned int j = 0; j < _number_cross_slip_planes; ++j)
      Moose::out << " is the slip direction number " << _cross_slip_familes[i][j] << std::endl;
  }
#endif
}

unsigned int
CrystalPlasticityStressUpdateBase::identifyCrossSlipFamily(const unsigned int index)
{
  for (unsigned int i = 0; i < _number_cross_slip_directions; ++i)
    for (unsigned int j = 0; j < _number_cross_slip_planes; ++j)
      if (_cross_slip_familes[i][j] == index)
        return i;

  // Should never reach this statement
  mooseError("The supplied slip system index is not among the slip system families sorted.");
}

void
CrystalPlasticityStressUpdateBase::calculateFlowDirection(const RankTwoTensor & crysrot)
{
  calculateSchmidTensor(
      _number_slip_systems, _slip_plane_normal, _slip_direction, _flow_direction[_qp], crysrot);
}

void
CrystalPlasticityStressUpdateBase::calculateSchmidTensor(
    const unsigned int & number_dislocation_systems,
    const DenseVector<Real> & plane_normal_vector,
    const DenseVector<Real> & direction_vector,
    std::vector<RankTwoTensor> & schmid_tensor,
    const RankTwoTensor & crysrot)
{
  DenseVector<Real> local_direction_vector(LIBMESH_DIM * number_dislocation_systems),
      local_plane_normal(LIBMESH_DIM * number_dislocation_systems);

  // Update slip direction and normal with crystal orientation
  for (unsigned int i = 0; i < number_dislocation_systems; ++i)
  {
    unsigned int system = i * LIBMESH_DIM;
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      local_direction_vector(system + j) = 0.0;
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        local_direction_vector(system + j) =
            local_direction_vector(system + j) + crysrot(j, k) * direction_vector(system + k);
      }
    }

    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      local_plane_normal(system + j) = 0.0;
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        local_plane_normal(system + j) =
            local_plane_normal(system + j) + crysrot(j, k) * plane_normal_vector(system + k);
    }
  }

  // Calculate Schmid tensor and resolved shear stresses
  for (unsigned int i = 0; i < number_dislocation_systems; ++i)
  {
    unsigned int system = i * LIBMESH_DIM;
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
      {
        schmid_tensor[i](j, k) =
            local_direction_vector(system + j) * local_plane_normal(system + k);
      }
  }
}

void
CrystalPlasticityStressUpdateBase::calculateShearStress(RankTwoTensor & pk2)
{
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
    _tau[_qp][i] = pk2.doubleContraction(_flow_direction[_qp][i]);
}

void
CrystalPlasticityStressUpdateBase::calculateTotalPlasticDeformationGradientDerivative(
    RankFourTensor & dfpinvdpk2, const RankTwoTensor & inverse_plastic_deformation_grad_old)
{
  std::vector<Real> dslip_dtau(_number_slip_systems, 0.0);
  std::vector<RankTwoTensor> dtaudpk2(_number_slip_systems);
  std::vector<RankTwoTensor> dfpinvdslip(_number_slip_systems);

  calculateConstitutiveSlipDerivative(dslip_dtau);

  for (unsigned int j = 0; j < _number_slip_systems; ++j)
  {
    dtaudpk2[j] = _flow_direction[_qp][j];
    dfpinvdslip[j] = -inverse_plastic_deformation_grad_old * _flow_direction[_qp][j];
    dfpinvdpk2 += (dfpinvdslip[j] * dslip_dtau[j] * _substep_dt).outerProduct(dtaudpk2[j]);
  }
}

void
CrystalPlasticityStressUpdateBase::calculateEquivalentSlipIncrement(
    RankTwoTensor & equivalent_slip_increment)
{
  // Sum up the slip increments to find the equivalent plastic strain due to slip
  for (unsigned int i = 0; i < _number_slip_systems; ++i)
    equivalent_slip_increment += _flow_direction[_qp][i] * _slip_increment[_qp][i] * _substep_dt;
}

void
CrystalPlasticityStressUpdateBase::setQp(const unsigned int & qp)
{
  _qp = qp;
}

void
CrystalPlasticityStressUpdateBase::setSubstepDt(const Real & substep_dt)
{
  _substep_dt = substep_dt;
}

bool
CrystalPlasticityStressUpdateBase::isConstitutiveStateVariableConverged(
    const std::vector<Real> & current_var,
    const std::vector<Real> & var_before_update,
    const std::vector<Real> & previous_substep_var,
    const Real & tolerance)
{
  // sometimes the state variable size may not equal to the number of slip systems
  unsigned int sz = current_var.size();
  mooseAssert(current_var.size() == sz, "Current variable size does not match");
  mooseAssert(var_before_update.size() == sz, "Variable before update size does not match");
  mooseAssert(previous_substep_var.size() == sz, "Previous substep variable size does not match");

  bool is_converged = true;

  Real diff_val = 0.0;
  Real abs_prev_substep_val = 0.0;
  for (unsigned int i = 0; i < sz; ++i)
  {
    diff_val = std::abs(var_before_update[i] - current_var[i]);
    abs_prev_substep_val = std::abs(previous_substep_var[i]);

    // set to false if the state variable is not converged
    if (abs_prev_substep_val < _zero_tol && diff_val > _zero_tol)
      is_converged = false;
    else if (abs_prev_substep_val > _zero_tol && diff_val > tolerance * abs_prev_substep_val)
      is_converged = false;
  }
  return is_converged;
}
