//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeGBMisorientationType.h"
#include "SolutionUserObject.h"

registerMooseObject("PhaseFieldApp", ComputeGBMisorientationType);

InputParameters
ComputeGBMisorientationType::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculate types of grain boundaries in a polycrystalline sample");
  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "The GrainTracker UserObject to get values from.");
  params.addRequiredParam<UserObjectName>("ebsd_reader", "The EBSDReader GeneralUserObject");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addParam<Real>("angle_threshold", 15, "Max LAGB Misorientation angle");
  return params;
}

ComputeGBMisorientationType::ComputeGBMisorientationType(const InputParameters & parameters)
  : Material(parameters),
    _grain_tracker(getUserObject<GrainTracker>("grain_tracker")),
    _ebsd_reader(getUserObject<EBSDReader>("ebsd_reader")),
    _op_num(coupledComponents("v")),
    _vals(coupledValues("v")),
    _angle_threshold(getParam<Real>("angle_threshold")),
    _gb_type(declareADProperty<Real>("gb_type"))
{
  getMisorientationAngles();
}

void
ComputeGBMisorientationType::computeQpProperties()
{
  // Find out the number of boundary unique_id and save them
  _gb_pairs.clear();
  _gb_op_pairs.clear();

  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());
  for (auto i : index_range(op_to_grains))
  {
    if (op_to_grains[i] == FeatureFloodCount::invalid_id)
      continue;

    _gb_pairs.push_back(_ebsd_reader.getFeatureID(op_to_grains[i]));
    _gb_op_pairs.push_back((*_vals[i])[_qp]);
  }

  // Compute GB type by the number of id
  _gb_type[_qp] = 0;
  switch (_gb_pairs.size())
  {
    case 0:
      break;
    case 1:
      break;
    case 2:
      // get type by Misorientation angle
      _gb_type[_qp] =
          ((_misorientation_angles[getLineNum(_gb_pairs[0], _gb_pairs[1])] < _angle_threshold) ? 1
                                                                                               : 2);
      break;
    default:
      // get continuous type at triple junction
      _gb_type[_qp] = getTripleJunctionType();
  }
}

// Function to output total line number of Misorientation angle file
unsigned int
ComputeGBMisorientationType::getTotalLineNum() const
{
  return _misorientation_angles.size();
}

// Function to output specific line number in Misorientation angle file
unsigned int
ComputeGBMisorientationType::getLineNum(unsigned int grain_i, unsigned int grain_j)
{
  if (grain_i > grain_j)
    return grain_j + (grain_i - 1) * grain_i / 2;
  else
    return grain_i + (grain_j - 1) * grain_j / 2;
}

// Function to calculate the GB type in Triple junction
Real
ComputeGBMisorientationType::getTripleJunctionType()
{
  unsigned int lagb_num = 0;
  unsigned int hagb_num = 0;
  Real ratio_base = 0.0;
  Real ratio_lagb = 0.0;
  for (unsigned int i = 1; i < _gb_pairs.size(); ++i)
  {
    for (unsigned int j = 0; j < i; ++j)
    {
      ratio_base += (_gb_op_pairs[i] * _gb_op_pairs[i] * _gb_op_pairs[j] * _gb_op_pairs[j]);
      if (_misorientation_angles[getLineNum(_gb_pairs[j], _gb_pairs[i])] < _angle_threshold)
      {
        lagb_num += 1;
        ratio_lagb += (_gb_op_pairs[i] * _gb_op_pairs[i] * _gb_op_pairs[j] * _gb_op_pairs[j]);
      }
      else
        hagb_num += 1;
    }
  }
  if (lagb_num == 0)
    return 2;
  else if (hagb_num == 0)
    return 1;
  else
    return 2 - ratio_lagb / ratio_base;
}

// Function to convert symmetry matrix to quaternion form
void
ComputeGBMisorientationType::rotationSymmetryToQuaternion(const double O[3][3],
                                                          Eigen::Quaternion<Real> & q)
{
  double q4 = 0;
  if ((1 + O[0][0] + O[1][1] + O[2][2]) > 0)
  {
    q4 = sqrt(1 + O[0][0] + O[1][1] + O[2][2]) / 2;
    q.w() = q4;
    q.x() = (O[2][1] - O[1][2]) / (4 * q4);
    q.y() = (O[0][2] - O[2][0]) / (4 * q4);
    q.z() = (O[1][0] - O[0][1]) / (4 * q4);
  }
  else if ((1 + O[0][0] - O[1][1] - O[2][2]) > 0)
  {
    q4 = sqrt(1 + O[0][0] - O[1][1] - O[2][2]) / 2;
    q.w() = (O[2][1] - O[1][2]) / (4 * q4);
    q.x() = q4;
    q.y() = (O[1][0] + O[0][1]) / (4 * q4);
    q.z() = (O[0][2] + O[2][0]) / (4 * q4);
  }
  else if ((1 - O[0][0] + O[1][1] - O[2][2]) > 0)
  {
    q4 = sqrt(1 - O[0][0] + O[1][1] - O[2][2]) / 2;
    q.w() = (O[0][2] - O[2][0]) / (4 * q4);
    q.x() = (O[1][0] + O[0][1]) / (4 * q4);
    q.y() = q4;
    q.z() = (O[2][1] + O[1][2]) / (4 * q4);
  }
  else if ((1 - O[0][0] - O[1][1] + O[2][2]) > 0)
  {
    q4 = sqrt(1 - O[0][0] - O[1][1] + O[2][2]) / 2;
    q.w() = (O[1][0] - O[0][1]) / (4 * q4);
    q.x() = (O[0][2] + O[2][0]) / (4 * q4);
    q.y() = (O[2][1] + O[1][2]) / (4 * q4);
    q.z() = q4;
  }
}

// Function to define the symmetry operator
void
ComputeGBMisorientationType::defineSymmetryOperator()
{
  // grow by number of symmetric operators
  _sym_quat.resize(_o_sym);

  // cubic symmetry
  double sym_rotation[24][3][3] = {
      {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},    {{1, 0, 0}, {0, -1, 0}, {0, 0, -1}},
      {{1, 0, 0}, {0, 0, -1}, {0, 1, 0}},   {{1, 0, 0}, {0, 0, 1}, {0, -1, 0}},
      {{-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},  {{-1, 0, 0}, {0, -1, 0}, {0, 0, 1}},
      {{-1, 0, 0}, {0, 0, -1}, {0, -1, 0}}, {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
      {{0, 1, 0}, {-1, 0, 0}, {0, 0, 1}},   {{0, 1, 0}, {0, 0, -1}, {-1, 0, 0}},
      {{0, 1, 0}, {1, 0, 0}, {0, 0, -1}},   {{0, 1, 0}, {0, 0, 1}, {1, 0, 0}},
      {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}},   {{0, -1, 0}, {0, 0, -1}, {1, 0, 0}},
      {{0, -1, 0}, {-1, 0, 0}, {0, 0, -1}}, {{0, -1, 0}, {0, 0, 1}, {-1, 0, 0}},
      {{0, 0, 1}, {0, 1, 0}, {-1, 0, 0}},   {{0, 0, 1}, {1, 0, 0}, {0, 1, 0}},
      {{0, 0, 1}, {0, -1, 0}, {1, 0, 0}},   {{0, 0, 1}, {-1, 0, 0}, {0, -1, 0}},
      {{0, 0, -1}, {0, 1, 0}, {1, 0, 0}},   {{0, 0, -1}, {-1, 0, 0}, {0, 1, 0}},
      {{0, 0, -1}, {0, -1, 0}, {-1, 0, 0}}, {{0, 0, -1}, {1, 0, 0}, {0, -1, 0}}};

  // initialize global operator
  for (int o = 0; o < _o_sym; o++)
    rotationSymmetryToQuaternion(sym_rotation[o], _sym_quat[o]);
}

// Function to return the misorientation of two quaternions
double
ComputeGBMisorientationType::getMisorientationFromQuaternion(const Eigen::Quaternion<Real> qi,
                                                             const Eigen::Quaternion<Real> qj)
{
  Real miso0, misom = 2.0 * libMesh::pi;
  Eigen::Quaternion<Real> q, qib, qjb, qmin;
  qmin.w() = 0;
  qmin.x() = 0;
  qmin.y() = 0;
  qmin.z() = 0;

  for (int o1 = 0; o1 < _o_sym; o1++)
  {
    for (int o2 = 0; o2 < _o_sym; o2++)
    {
      qib = _sym_quat[o1] * qi;
      qjb = _sym_quat[o2] * qj;

      // j-grain conjugate quaternion
      qjb.x() = -qjb.x();
      qjb.y() = -qjb.y();
      qjb.z() = -qjb.z();
      q = qib * qjb;
      miso0 = round(2 * std::acos(q.w()) * 1e5) / 1e5;

      if (miso0 > libMesh::pi)
        miso0 = miso0 - 2 * libMesh::pi;
      if (std::abs(miso0) < misom)
      {
        misom = std::abs(miso0);
        qmin = q;
      }
    }
  }

  miso0 = 2 * std::acos(qmin.w());
  if (miso0 > libMesh::pi)
    miso0 = miso0 - 2 * libMesh::pi;

  return std::abs(miso0);
}

// Get the Misorientation angle
void
ComputeGBMisorientationType::getMisorientationAngles()
{
  // Initialize symmetry operator as quaternion vectors
  defineSymmetryOperator();
  // Initialize parameters to calculate misorientation
  const auto grain_num = _ebsd_reader.getGrainNum();
  _euler_angle.resize(grain_num);
  _quat_angle.resize(grain_num);

  // Get Euler Angle of Orientation
  for (const auto i : make_range(grain_num))
  {
    auto grain_id = _ebsd_reader.getFeatureID(i);
    _euler_angle[grain_id] = _ebsd_reader.getEulerAngles(i);
    _quat_angle[grain_id] = _euler_angle[grain_id].toQuaternion();
  }

  for (const auto j : make_range(std::make_unsigned_t<int>(1), grain_num))
  {
    for (const auto i : make_range(j))
    {
      Real theta = getMisorientationFromQuaternion(_quat_angle[i], _quat_angle[j]);
      _misorientation_angles.push_back(theta / libMesh::pi * 180);
    }
  }
}
