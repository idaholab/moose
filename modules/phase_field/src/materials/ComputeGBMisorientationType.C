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

#define MY_PI 3.14159265358979323846  // pi
#define MY_2PI 6.28318530717958647692 // 2pi

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
  std::vector<unsigned int> gb_pairs;
  std::vector<Real> gb_op_pairs;
  MooseIndex(_vals) val_index = -1;

  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());
  for (auto & unique_id : op_to_grains)
  {
    val_index += 1;
    if (unique_id == FeatureFloodCount::invalid_id)
      continue;

    auto grain_id = _ebsd_reader.getFeatureID(unique_id);

    gb_pairs.push_back(grain_id);
    gb_op_pairs.push_back((*_vals[val_index])[_qp]);
  }

  // Compute GB type by the number of id
  _gb_type[_qp] = 0;
  switch (gb_pairs.size())
  {
    case 0:
      break;
    case 1:
      break;
    case 2:
      // get type by Misorientation angle
      _gb_type[_qp] =
          ((_misorientation_angles[getLineNum(gb_pairs[0], gb_pairs[1])] < _angle_threshold) ? 1
                                                                                             : 2);
      break;
    default:
      // get continuous type at triple junction
      _gb_type[_qp] = getTripleJunctionType(gb_pairs, gb_op_pairs);
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
ComputeGBMisorientationType::getTripleJunctionType(std::vector<unsigned int> gb_pairs,
                                                   std::vector<Real> gb_op_pairs)
{
  unsigned int lagb_num = 0;
  unsigned int hagb_num = 0;
  Real ratio_base = 0.0;
  Real ratio_lagb = 0.0;
  for (unsigned int i = 1; i < gb_pairs.size(); ++i)
  {
    for (unsigned int j = 0; j < i; ++j)
    {
      ratio_base += (gb_op_pairs[i] * gb_op_pairs[i] * gb_op_pairs[j] * gb_op_pairs[j]);
      if (_misorientation_angles[getLineNum(gb_pairs[j], gb_pairs[i])] < _angle_threshold)
      {
        lagb_num += 1;
        ratio_lagb += (gb_op_pairs[i] * gb_op_pairs[i] * gb_op_pairs[j] * gb_op_pairs[j]);
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
ComputeGBMisorientationType::rotationSymmetryToQuaternion(const double O[3][3], double q[4])
{
  double q4 = 0;
  if ((1 + O[0][0] + O[1][1] + O[2][2]) > 0)
  {
    q4 = sqrt(1 + O[0][0] + O[1][1] + O[2][2]) / 2;
    q[0] = q4;
    q[1] = (O[2][1] - O[1][2]) / (4 * q4);
    q[2] = (O[0][2] - O[2][0]) / (4 * q4);
    q[3] = (O[1][0] - O[0][1]) / (4 * q4);
  }
  else if ((1 + O[0][0] - O[1][1] - O[2][2]) > 0)
  {
    q4 = sqrt(1 + O[0][0] - O[1][1] - O[2][2]) / 2;
    q[0] = (O[2][1] - O[1][2]) / (4 * q4);
    q[1] = q4;
    q[2] = (O[1][0] + O[0][1]) / (4 * q4);
    q[3] = (O[0][2] + O[2][0]) / (4 * q4);
  }
  else if ((1 - O[0][0] + O[1][1] - O[2][2]) > 0)
  {
    q4 = sqrt(1 - O[0][0] + O[1][1] - O[2][2]) / 2;
    q[0] = (O[0][2] - O[2][0]) / (4 * q4);
    q[1] = (O[1][0] + O[0][1]) / (4 * q4);
    q[2] = q4;
    q[3] = (O[2][1] + O[1][2]) / (4 * q4);
  }
  else if ((1 - O[0][0] - O[1][1] + O[2][2]) > 0)
  {
    q4 = sqrt(1 - O[0][0] - O[1][1] + O[2][2]) / 2;
    q[0] = (O[1][0] - O[0][1]) / (4 * q4);
    q[1] = (O[0][2] + O[2][0]) / (4 * q4);
    q[2] = (O[2][1] + O[1][2]) / (4 * q4);
    q[3] = q4;
  }
}

// Function to define the symmetry operator
void
ComputeGBMisorientationType::defineSymmetryOperator(double *** sym)
{
  // grow by number of symmetric operators
  (*sym) = new double *[o_sym];

  // grow for symmetry quaternion vectors
  for (int o = 0; o < o_sym; o++)
    (*sym)[o] = new double[4];

  // buffer for quaternion
  double q[4];

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
  for (int o = 0; o < o_sym; o++)
  {
    rotationSymmetryToQuaternion(sym_rotation[o], q);
    for (int i = 0; i < 4; i++)
      (*sym)[o][i] = q[i];
  }
}

// Function to convert grain Euler angles to quaternion vector
void
ComputeGBMisorientationType::eulerOrientationToQuaternion(int grain_id)
{
  quat_angle[grain_id][0] = cos(euler_angle[grain_id][1] / 2.) *
                            cos((euler_angle[grain_id][0] + euler_angle[grain_id][2]) / 2.);
  quat_angle[grain_id][1] = sin(euler_angle[grain_id][1] / 2.) *
                            cos((euler_angle[grain_id][0] - euler_angle[grain_id][2]) / 2.);
  quat_angle[grain_id][2] = sin(euler_angle[grain_id][1] / 2.) *
                            sin((euler_angle[grain_id][0] - euler_angle[grain_id][2]) / 2.);
  quat_angle[grain_id][3] = cos(euler_angle[grain_id][1] / 2.) *
                            sin((euler_angle[grain_id][0] + euler_angle[grain_id][2]) / 2.);
}

// Function to multiply quaternions and update
void
ComputeGBMisorientationType::getQuaternionProduct(const double qi[4],
                                                  const double qj[4],
                                                  double q[4])
{
  q[0] = qi[0] * qj[0] - qi[1] * qj[1] - qi[2] * qj[2] - qi[3] * qj[3];
  q[1] = qi[0] * qj[1] + qi[1] * qj[0] + qi[2] * qj[3] - qi[3] * qj[2];
  q[2] = qi[0] * qj[2] - qi[1] * qj[3] + qi[2] * qj[0] + qi[3] * qj[1];
  q[3] = qi[0] * qj[3] + qi[1] * qj[2] - qi[2] * qj[1] + qi[3] * qj[0];
}

// Function to return the misorientation of two quaternions
double
ComputeGBMisorientationType::getMisorientationFromQuaternion(const double qi[4], const double qj[4])
{
  double miso0, misom = MY_2PI;
  double q[4], qib[4], qjb[4], qmin[4] = {0, 0, 0, 0};

  for (int o1 = 0; o1 < o_sym; o1++)
  {
    for (int o2 = 0; o2 < o_sym; o2++)
    {
      getQuaternionProduct(sym_quat[o1], qi, qib);
      getQuaternionProduct(sym_quat[o2], qj, qjb);

      // j-grain conjugate quaternion
      qjb[1] = -qjb[1];
      qjb[2] = -qjb[2];
      qjb[3] = -qjb[3];
      getQuaternionProduct(qib, qjb, q);
      miso0 = round(2 * acos(q[0]) * 1e5) / 1e5;

      if (miso0 > MY_PI)
        miso0 = miso0 - MY_2PI;
      if (fabs(miso0) < misom)
      {
        misom = fabs(miso0);
        qmin[0] = q[0];
        qmin[1] = q[1];
        qmin[2] = q[2];
        qmin[3] = q[3];
      }
    }
  }

  miso0 = 2 * acos(qmin[0]);
  if (miso0 > MY_PI)
    miso0 = miso0 - MY_2PI;

  return fabs(miso0);
}

// Get the Misorientation angle
void
ComputeGBMisorientationType::getMisorientationAngles()
{
  // Initialize symmetry operator as quaternion vectors
  defineSymmetryOperator(&sym_quat);
  int grain_num = _ebsd_reader.getGrainNum();
  euler_angle.assign(grain_num, std::vector<double>(3));
  quat_angle.assign(grain_num, std::vector<double>(4));

  // Get Euler Angle of Orientation
  for (int i = 0; i < grain_num; i++)
  {
    const EulerAngles & d = _ebsd_reader.getEulerAngles(i);
    auto grain_id = _ebsd_reader.getFeatureID(i);
    euler_angle[grain_id][0] = d.phi1 / 180 * MY_PI;
    euler_angle[grain_id][1] = d.Phi / 180 * MY_PI;
    euler_angle[grain_id][2] = d.phi2 / 180 * MY_PI;
    eulerOrientationToQuaternion(grain_id);
  }

  double qi[4], qj[4];
  for (int j = 1; j < grain_num; j++)
  {
    for (int i = 0; i < j; i++)
    {
      qi[0] = quat_angle[j][0];
      qi[1] = quat_angle[j][1];
      qi[2] = quat_angle[j][2];
      qi[3] = quat_angle[j][3];
      qj[0] = quat_angle[i][0];
      qj[1] = quat_angle[i][1];
      qj[2] = quat_angle[i][2];
      qj[3] = quat_angle[i][3];
      double theta = getMisorientationFromQuaternion(qj, qi);
      _misorientation_angles.push_back(theta / MY_PI * 180);
    }
  }
}
