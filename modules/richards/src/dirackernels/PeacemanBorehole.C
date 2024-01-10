//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeacemanBorehole.h"
#include "RotationMatrix.h"

#include <fstream>

InputParameters
PeacemanBorehole::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addRequiredParam<FunctionName>(
      "character",
      "If zero then borehole does nothing.  If positive the borehole acts as a sink "
      "(production well) for porepressure > borehole pressure, and does nothing "
      "otherwise.  If negative the borehole acts as a source (injection well) for "
      "porepressure < borehole pressure, and does nothing otherwise.  The flow rate "
      "to/from the borehole is multiplied by |character|, so usually character = +/- "
      "1, but you can specify other quantities to provide an overall scaling to the "
      "flow if you like.");
  params.addRequiredParam<Real>("bottom_pressure", "Pressure at the bottom of the borehole");
  params.addRequiredParam<RealVectorValue>(
      "unit_weight",
      "(fluid_density*gravitational_acceleration) as a vector pointing downwards.  "
      "Note that the borehole pressure at a given z position is bottom_pressure + "
      "unit_weight*(p - p_bottom), where p=(x,y,z) and p_bottom=(x,y,z) of the "
      "bottom point of the borehole.  If you don't want bottomhole pressure to vary "
      "in the borehole just set unit_weight=0.  Typical value is = (0,0,-1E4)");
  params.addRequiredParam<std::string>(
      "point_file",
      "The file containing the borehole radii and coordinates of the point sinks "
      "that approximate the borehole.  Each line in the file must contain a "
      "space-separated radius and coordinate.  Ie r x y z.  The last point in the "
      "file is defined as the borehole bottom, where the borehole pressure is "
      "bottom_pressure.  If your file contains just one point, you must also specify "
      "the borehole_length and borehole_direction.  Note that you will get "
      "segementation faults if your points do not lie within your mesh!");
  params.addRequiredParam<UserObjectName>(
      "SumQuantityUO",
      "User Object of type=RichardsSumQuantity in which to place the total "
      "outflow from the borehole for each time step.");
  params.addParam<Real>("re_constant",
                        0.28,
                        "The dimensionless constant used in evaluating the borehole effective "
                        "radius.  This depends on the meshing scheme.  Peacemann "
                        "finite-difference calculations give 0.28, while for rectangular finite "
                        "elements the result is closer to 0.1594.  (See  Eqn(4.13) of Z Chen, Y "
                        "Zhang, Well flow models for various numerical methods, Int J Num "
                        "Analysis and Modeling, 3 (2008) 375-388.)");
  params.addParam<Real>("well_constant",
                        -1.0,
                        "Usually this is calculated internally from the element geometry, the "
                        "local borehole direction and segment length, and the permeability.  "
                        "However, if this parameter is given as a positive number then this "
                        "number is used instead of the internal calculation.  This speeds up "
                        "computation marginally.  re_constant becomes irrelevant");
  params.addRangeCheckedParam<Real>(
      "borehole_length",
      0.0,
      "borehole_length>=0",
      "Borehole length.  Note this is only used if there is only one point in the point_file.");
  params.addParam<RealVectorValue>(
      "borehole_direction",
      RealVectorValue(0, 0, 1),
      "Borehole direction.  Note this is only used if there is only one point in the point_file.");
  params.addClassDescription("Approximates a borehole in the mesh using the Peaceman approach, ie "
                             "using a number of point sinks with given radii whose positions are "
                             "read from a file");
  return params;
}

PeacemanBorehole::PeacemanBorehole(const InputParameters & parameters)
  : DiracKernel(parameters),
    _re_constant(getParam<Real>("re_constant")),
    _well_constant(getParam<Real>("well_constant")),
    _borehole_length(getParam<Real>("borehole_length")),
    _borehole_direction(getParam<RealVectorValue>("borehole_direction")),
    _point_file(getParam<std::string>("point_file")),
    _character(getFunction("character")),
    _p_bot(getParam<Real>("bottom_pressure")),
    _unit_weight(getParam<RealVectorValue>("unit_weight")),
    _total_outflow_mass(
        const_cast<RichardsSumQuantity &>(getUserObject<RichardsSumQuantity>("SumQuantityUO")))
{
  // zero the outflow mass
  _total_outflow_mass.zero();

  // open file
  std::ifstream file(_point_file.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _point_file + "' from a Peaceman-type Borehole.");

  // construct the arrays of radius, x, y and z
  std::vector<Real> scratch;
  while (parseNextLineReals(file, scratch))
  {
    if (scratch.size() >= 2)
    {
      _rs.push_back(scratch[0]);
      _xs.push_back(scratch[1]);
      if (scratch.size() >= 3)
        _ys.push_back(scratch[2]);
      else
        _ys.push_back(0.0);
      if (scratch.size() >= 4)
        _zs.push_back(scratch[3]);
      else
        _zs.push_back(0.0);
    }
  }

  file.close();

  const int num_pts = _zs.size();
  _bottom_point(0) = _xs[num_pts - 1];
  _bottom_point(1) = _ys[num_pts - 1];
  _bottom_point(2) = _zs[num_pts - 1];

  // construct the line-segment lengths between each point
  _half_seg_len.resize(std::max(num_pts - 1, 1));
  for (unsigned int i = 0; i + 1 < _xs.size(); ++i)
  {
    _half_seg_len[i] =
        0.5 * std::sqrt(std::pow(_xs[i + 1] - _xs[i], 2) + std::pow(_ys[i + 1] - _ys[i], 2) +
                        std::pow(_zs[i + 1] - _zs[i], 2));
    if (_half_seg_len[i] == 0)
      mooseError("Peaceman-type borehole has a zero-segment length at (x,y,z) = ",
                 _xs[i],
                 " ",
                 _ys[i],
                 " ",
                 _zs[i],
                 "\n");
  }
  if (num_pts == 1)
    _half_seg_len[0] = _borehole_length;

  // construct the rotation matrix needed to rotate the permeability
  _rot_matrix.resize(std::max(num_pts - 1, 1));
  for (unsigned int i = 0; i + 1 < _xs.size(); ++i)
  {
    const RealVectorValue v2(_xs[i + 1] - _xs[i], _ys[i + 1] - _ys[i], _zs[i + 1] - _zs[i]);
    _rot_matrix[i] = RotationMatrix::rotVecToZ(v2);
  }
  if (num_pts == 1)
    _rot_matrix[0] = RotationMatrix::rotVecToZ(_borehole_direction);
}

bool
PeacemanBorehole::parseNextLineReals(std::ifstream & ifs, std::vector<Real> & myvec)
// reads a space-separated line of floats from ifs and puts in myvec
{
  std::string line;
  myvec.clear();
  bool gotline(false);
  if (getline(ifs, line))
  {
    gotline = true;

    // Harvest floats separated by whitespace
    std::istringstream iss(line);
    Real f;
    while (iss >> f)
    {
      myvec.push_back(f);
    }
  }
  return gotline;
}

void
PeacemanBorehole::addPoints()
{
  // This function gets called just before the DiracKernel is evaluated
  // so this is a handy place to zero this out.
  _total_outflow_mass.zero();

  // Add point using the unique ID "i", let the DiracKernel take
  // care of the caching.  This should be fast after the first call,
  // as long as the points don't move around.
  for (unsigned int i = 0; i < _zs.size(); i++)
    addPoint(Point(_xs[i], _ys[i], _zs[i]), i);
}

Real
PeacemanBorehole::wellConstant(const RealTensorValue & perm,
                               const RealTensorValue & rot,
                               const Real & half_len,
                               const Elem * ele,
                               const Real & rad)
// Peaceman's form for the borehole well constant
{
  if (_well_constant > 0)
    return _well_constant;

  // rot_perm has its "2" component lying along the half segment
  // we want to determine the eigenvectors of rot(0:1, 0:1), since, when
  // rotated back to the original frame we will determine the element
  // lengths along these directions
  const RealTensorValue rot_perm = (rot * perm) * rot.transpose();
  const Real trace2D = rot_perm(0, 0) + rot_perm(1, 1);
  const Real det2D = rot_perm(0, 0) * rot_perm(1, 1) - rot_perm(0, 1) * rot_perm(1, 0);
  const Real sq = std::sqrt(std::max(0.25 * trace2D * trace2D - det2D,
                                     0.0)); // the std::max accounts for wierdo precision loss
  const Real eig_val1 = 0.5 * trace2D + sq;
  const Real eig_val2 = 0.5 * trace2D - sq;
  RealVectorValue eig_vec1, eig_vec2;
  if (sq > std::abs(trace2D) * 1E-7) // matrix is not a multiple of the identity (1E-7 accounts for
                                     // precision in a crude way)
  {
    if (rot_perm(1, 0) != 0)
    {
      eig_vec1(0) = eig_val1 - rot_perm(1, 1);
      eig_vec1(1) = rot_perm(1, 0);
      eig_vec2(0) = eig_val2 - rot_perm(1, 1);
      eig_vec2(1) = rot_perm(1, 0);
    }
    else if (rot_perm(0, 1) != 0)
    {
      eig_vec1(0) = rot_perm(0, 1);
      eig_vec1(1) = eig_val1 - rot_perm(0, 0);
      eig_vec2(0) = rot_perm(0, 1);
      eig_vec2(1) = eig_val2 - rot_perm(0, 0);
    }
    else // off diagonal terms are both zero
    {
      eig_vec1(0) = 1;
      eig_vec2(1) = 1;
    }
  }
  else // matrix is basically a multiple of the identity
  {
    eig_vec1(0) = 1;
    eig_vec2(1) = 1;
  }

  // finally, rotate these to original frame and normalise
  eig_vec1 = rot.transpose() * eig_vec1;
  eig_vec1 /= std::sqrt(eig_vec1 * eig_vec1);
  eig_vec2 = rot.transpose() * eig_vec2;
  eig_vec2 /= std::sqrt(eig_vec2 * eig_vec2);

  // find the "length" of the element in these directions
  // TODO - probably better to use variance than max&min
  Real max1 = eig_vec1 * ele->point(0);
  Real max2 = eig_vec2 * ele->point(0);
  Real min1 = max1;
  Real min2 = max2;
  Real proj;
  for (unsigned int i = 1; i < ele->n_nodes(); i++)
  {
    proj = eig_vec1 * ele->point(i);
    max1 = (max1 < proj) ? proj : max1;
    min1 = (min1 < proj) ? min1 : proj;

    proj = eig_vec2 * ele->point(i);
    max2 = (max2 < proj) ? proj : max2;
    min2 = (min2 < proj) ? min2 : proj;
  }
  const Real ll1 = max1 - min1;
  const Real ll2 = max2 - min2;

  const Real r0 = _re_constant *
                  std::sqrt(std::sqrt(eig_val1 / eig_val2) * std::pow(ll2, 2) +
                            std::sqrt(eig_val2 / eig_val1) * std::pow(ll1, 2)) /
                  (std::pow(eig_val1 / eig_val2, 0.25) + std::pow(eig_val2 / eig_val1, 0.25));

  const Real effective_perm = std::sqrt(det2D);

  const Real halfPi = acos(0.0);

  if (r0 <= rad)
    mooseError("The effective element size (about 0.2-times-true-ele-size) for an element "
               "containing a Peaceman-type borehole must be (much) larger than the borehole radius "
               "for the Peaceman formulation to be correct.  Your element has effective size ",
               r0,
               " and the borehole radius is ",
               rad,
               "\n");

  return 4 * halfPi * effective_perm * half_len / std::log(r0 / rad);
}
