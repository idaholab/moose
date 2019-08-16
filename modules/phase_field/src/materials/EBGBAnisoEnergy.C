//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EBGBAnisoEnergy.h"
#include <iostream>

registerMooseObject("PhaseFieldApp", EBGBAnisoEnergy);

template <>
InputParameters
validParams<EBGBAnisoEnergy>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper<> >();
  params.addClassDescription("Calculate value of grain boundaries in a polycrystalline sample");
  params.addParam<int>("s",20,"Sharpness of Grain Boundary edge");
  params.addParam<int>("t",20,"Thickness of Grain Boundary");
  params.addRequiredCoupledVarWithAutoBuild(
      "v","var_name_base","op_num","Array of coupled variables");
  MooseEnum materials("Cu Al Au Ni", "Cu");
  params.addParam<MooseEnum>("material", materials, "Which material parameters to use");
  params.addParam<MaterialPropertyName>("op_to_grain_name",
                                        "op_to_grain",
                                        "Gives the grain associated with order parameter according to location");
  return params;
}

EBGBAnisoEnergy::EBGBAnisoEnergy(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper<>(parameters),
    _op_num(coupledComponents("v")),
    _thickness(getParam<int>("t")),
    _sharpness(getParam<int>("s")),
    _geom100(5),
    _geom110(5),
    _geom111(5),
    _axes100(3),
    _dirs100(3),
    _axes110(6),
    _dirs110(6),
    _axes111(4),
    _dirs111(4),
    _rot_X_p90(RotationTensor::XAXIS, 90.0),
    _rot_Y_p90(RotationTensor::YAXIS, 90.0),
    _rot_Z_p90(RotationTensor::ZAXIS, 90.0),
    _rot_Z_n90(RotationTensor::ZAXIS, -90.0),
    _symmetry_variants(24),
    _epsilon(1e-6),
    _orientation_matrix(_op_num),
    _par_vec(42),
    _op_to_grain_name(getParam<MaterialPropertyName>("op_to_grain_name")),
    _material(getParam<MooseEnum>("material").getEnum<Material>())
{
  setParVec();

  setAxes100();
  setAxes110();
  setAxes111();

  EBVectorFunction axis100(1, 0, 0);

  EBTerm::EBTermVector _vals = EBTerm::CreateEBTermVector("v",_op_num);
  EBVectorFunction::EBVectorVector _grad_vals = EBVectorFunction::CreateEBVectorVector("v", _op_num);

  for(unsigned int i = 0; i < _op_num; ++i)
  {
    _orientation_matrix[i] = setEBMaterialPropertyRankTwoTensor(_op_to_grain_name + std::to_string(i));
  }

  EBVectorFunction gbdir;
  EBTerm grad_norm;
  EBMatrixFunction rotation_matrix(3,3);
  EBTerm GBen;
  EBTerm _GB_location;
  EBTerm _GB_energy(0);

  for(unsigned int i = 0; i < _op_num; ++i)
  {
    EBTerm grad_norm = (_grad_vals)[i].norm();
    EBVectorFunction gbdir = (_grad_vals)[i] / grad_norm;

    rotation_matrix = ExpressionBuilder::EBMatrixFunction::rotVec1ToVec2(gbdir, axis100);
    std::cout << rotation_matrix.rowNum() << rotation_matrix.colNum() << std::endl;
    for(unsigned int j = i + 1; j < _op_num; ++j)
    {
      GBen = conditional((_orientation_matrix[i])[0][0] <= 1 && (_orientation_matrix[j])[0][0] <= 1,
                         gB5DOF(rotation_matrix * _orientation_matrix[i],
                                rotation_matrix * _orientation_matrix[j]),
                         0);

      _GB_location = 1.0 + pow(_vals[i],_thickness) + pow(_vals[j],_thickness);
      _GB_location = pow(-1.0 + 2.0/(_GB_location),_sharpness);

      _GB_energy = _GB_energy + GBen * _GB_location;
    }
  }

  functionParse(_GB_energy);
}

void
EBGBAnisoEnergy::setParVec()
{
  // _par_vec can be defined for just Al and Cu and then interpolated for the
  // other FCC metals then the energy scaling parameter, _par_vec[0], defined seperately
  const Real par42Al[42] = {
      0.405204179289160, 0.738862004021890, 0.351631012630026, 2.40065811939667,
      1.34694439281655,  0.352260396651516, 0.602137375062785, 1.58082498976078,
      0.596442399566661, 1.30981422643602,  3.21443408257354,  0.893016409093743,
      0.835332505166333, 0.933176738717594, 0.896076948651935, 0.775053293192055,
      0.391719619979054, 0.782601780600192, 0.678572601273508, 1.14716256515278,
      0.529386201144101, 0.909044736601838, 0.664018011430602, 0.597206897283586,
      0.200371750006251, 0.826325891814124, 0.111228512469435, 0.664039563157148,
      0.241537262980083, 0.736315075146365, 0.514591177241156, 1.73804335876546,
      3.04687038671309,  1.48989831680317,  0.664965104218438, 0.495035051289975,
      0.495402996460658, 0.468878130180681, 0.836548944799803, 0.619285521065571,
      0.844685390948170, 1.02295427618256};
  const Real par42Cu[42] = {
      0.405204179289160, 0.738862004021890, 0.351631012630026,  2.40065811939667,
      1.34694439281655,  3.37892632736175,  0.602137375062785,  1.58082498976078,
      0.710489498577995, 0.737834049784765, 3.21443408257354,   0.893016409093743,
      0.835332505166333, 0.933176738717594, 0.896076948651935,  0.775053293192055,
      0.509781056492307, 0.782601780600192, 0.762160812499734,  1.10473084066580,
      0.529386201144101, 0.909044736601838, 0.664018011430602,  0.597206897283586,
      0.200371750006251, 0.826325891814124, 0.0226010533470218, 0.664039563157148,
      0.297920289861751, 0.666383447163744, 0.514591177241156,  1.73804335876546,
      2.69805148576400,  1.95956771207484,  0.948894352912787,  0.495035051289975,
      0.301975031994664, 0.574050577702240, 0.836548944799803,  0.619285521065571,
      0.844685390948170, 0.0491040633104212};

  Real AlCuParameter;
  Real eRGB;
  if (_material == Material::Ni)
  {
    eRGB = 1.44532834613925;
    AlCuParameter = 0.767911805073948;
  }
  else if (_material == Material::Al)
  {
    eRGB = 0.547128733614891;
    AlCuParameter = 0;
  }
  else if (_material == Material::Au)
  {
    eRGB = 0.529912885175204;
    AlCuParameter = 0.784289766313152;
  }
  else if (_material == Material::Cu)
  {
    eRGB = 1.03669431227427;
    AlCuParameter = 1;
  }
  else
    mooseError("Unknown material.");

  _par_vec[0] = eRGB;
  for (unsigned int i = 1; i < 43; ++i)
    _par_vec[i] = par42Al[i - 1] + AlCuParameter * (par42Cu[i - 1] - par42Al[i - 1]);
}

void
EBGBAnisoEnergy::setAxes100()
{
  // define 100 axes, normalize
  _axes100[0] = RealVectorValue(1, 0, 0);
  _axes100[1] = RealVectorValue(0, 1, 0);
  _axes100[2] = RealVectorValue(0, 0, 1);

  // define crystal direction
  _dirs100[0] = RealVectorValue(0, 1, 0);
  _dirs100[1] = RealVectorValue(0, 0, 1);
  _dirs100[2] = RealVectorValue(1, 0, 0);
}

void
EBGBAnisoEnergy::setAxes110()
{
  const Real s2 = 1.0 / std::sqrt(2.0);

  // define the 110 axes, normalize
  _axes110[0] = RealVectorValue(s2, s2, 0);
  _axes110[1] = RealVectorValue(s2, -s2, 0);
  _axes110[2] = RealVectorValue(s2, 0, s2);
  _axes110[3] = RealVectorValue(s2, 0, -s2);
  _axes110[4] = RealVectorValue(0, s2, s2);
  _axes110[5] = RealVectorValue(0, s2, -s2);

  // define the crystal direction perpendicular to each rotation axis.
  // The formalism demands that this be an axis of at least two-fold symmetry.
  _dirs110[0] = RealVectorValue(0, 0, 1);
  _dirs110[1] = RealVectorValue(0, 0, 1);
  _dirs110[2] = RealVectorValue(0, 1, 0);
  _dirs110[3] = RealVectorValue(0, 1, 0);
  _dirs110[4] = RealVectorValue(1, 0, 0);
  _dirs110[5] = RealVectorValue(1, 0, 0);
}

void
EBGBAnisoEnergy::setAxes111()
{
  const Real s3 = 1.0 / std::sqrt(3);
  const Real s2 = 1.0 / std::sqrt(2);

  // define the 111 axes,  normalize.
  _axes111[0] = RealVectorValue(s3, s3, s3);
  _axes111[1] = RealVectorValue(s3, -s3, -s3);
  _axes111[2] = RealVectorValue(-s3, s3, -s3);
  _axes111[3] = RealVectorValue(-s3, -s3, s3);

  // define crystal direction
  _dirs111[0] = RealVectorValue(s2, -s2, 0);
  _dirs111[1] = RealVectorValue(s2, s2, 0);
  _dirs111[2] = RealVectorValue(s2, s2, 0);
  _dirs111[3] = RealVectorValue(s2, -s2, 0);
}

ExpressionBuilder::EBTerm
EBGBAnisoEnergy::gB5DOF(EBMatrixFunction P, EBMatrixFunction S)
{
  distancesToSet(P, S, _geom100, _axes100, _dirs100);
  distancesToSet(P, S, _geom110, _axes110, _dirs110);
  distancesToSet(P, S, _geom111, _axes111, _dirs111);

  return weightedMeanEnergy();
}

void
EBGBAnisoEnergy::distancesToSet(const EBMatrixFunction & P, EBMatrixFunction & S,
                                std::vector<std::vector<EBTerm> > & geom,
                                const std::vector<EBVectorFunction> & axes,
                                const std::vector<EBVectorFunction> & dirs)
{
  unsigned int naxes = axes.size();

  const Real dismax = 1.0 - _epsilon;

  Real period = libMesh::pi * naxes / 6;

  _symmetry_variants[0] = S;

  for (unsigned int i = 1; i < 4; ++i) // Rotate three times around X by +90 degrees
  {
    std::cout << "working" << std::endl;
    _symmetry_variants[i] = _symmetry_variants[i-1] * _rot_X_p90;
  }
  for (unsigned int i = 4; i < 16; ++i) // Rotate three times around Y by +90 degrees
  {
    std::cout << "working" << std::endl;
    _symmetry_variants[i] = _symmetry_variants[i - 4] * _rot_Y_p90;
  }
  for (unsigned int i = 16; i < 20; ++i) // Rotate around Z by +90 degrees
  {
    std::cout << "working" << std::endl;
    _symmetry_variants[i] = _symmetry_variants[i - 16] * _rot_Z_p90;
  }
  for (unsigned int i = 20; i < 24; ++i) // Rotate around Z by -90 degrees
  {
    _symmetry_variants[i] = _symmetry_variants[i - 20] * _rot_Z_n90;
  }

  geom[0].resize(24 * naxes); // distances
  geom[1].resize(24 * naxes); // ksis
  geom[2].resize(24 * naxes); // etas
  geom[3].resize(24 * naxes); // phis
  geom[4].resize(24 * naxes); // zero out

  // Number of hits found so far
  unsigned int thisindex = 0;

  // Step through all combinations of symmetrically-equivalent axes and coset elements
  // _symmetry_variants
  for (unsigned int i = 0; i < naxes; ++i)
  {
    // Completing the orthonormal coordinate set.
    // theta1 and theta2 are defined in the plane spanned by (dir,dir2)
    EBVectorFunction dir2 = ExpressionBuilder::EBVectorFunction::cross(axes[i],dirs[i]);

    EBMatrixFunction R;

    // for each symmetry-related variant of the second grain
    for (unsigned int j = 0; j < 24; ++j)
    {
      R = _symmetry_variants[j].transpose() * P;
      std::cout << _symmetry_variants[j].rowNum() << _symmetry_variants[j].colNum() << std::endl;
      std::cout << P.rowNum() << P.colNum() << std::endl;
      // This rotates any vector in cube P into a vector in cube S
      // Calculation from here on out is much easier with quaternions.
      EBQuaternionFunction q(R);

      const EBTerm lq = sqrt(q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
      EBVectorFunction axi(q[1] / lq, q[2] / lq, q[3] / lq);

      // Rotation angle
      EBTerm psi = 2.0 * acos(q[0]);
      EBTerm dot_p = axi * axes[i];

      // Compute rotational distance from boundary P/S to the rotation set i.
      // This formula produces 2*sin(delta/2), where delta is the angle of closest approach.
      const EBTerm dis = 2 * sqrt(abs(1 - dot_p * dot_p)) * sin(psi / 2.0);

      // angle of rotation about ax that most closely approximates R
      EBTerm theta = 2 * atan(dot_p * tan(psi / 2.0));

      // compute the normal of the best-fitting GB in grain 1
      EBVectorFunction n1 = P[0];
      EBVectorFunction n2 = (_symmetry_variants[j])[0];

      std::vector<EBTerm> tempVector;
      tempVector.push_back(EBTerm(cos(theta / 2.0)));
      tempVector.push_back(sin(theta / 2.0) * (axes[i])[0]);
      tempVector.push_back(sin(theta / 2.0) * (axes[i])[1]);
      tempVector.push_back(sin(theta / 2.0) * (axes[i])[2]);
      EBQuaternionFunction q2(tempVector);

      // Rotation matrix that most closely approximates R
      EBMatrixFunction RA(q2);

      // from this point on we're dealing with the idealized rotation RA, not the original R
      EBVectorFunction m1 = n1 + RA.transpose() * n2;

      const EBTerm l = m1.norm();

      // halfway between the two normal vectors from the two grains
      m1 = m1 / l;

      // same representation in the other grain
      EBVectorFunction m2 = RA * m1;

      // compute the inclination angle for the common rotation axis and avoid NaN's if m1 and
      // axes[i] are exactly parallel
      const EBTerm m1_dot_axes = abs(m1 * axes[i]);
      EBTerm phi = conditional(m1_dot_axes > 1.0, 0.0, acos(m1_dot_axes));

      EBTerm theta1;
      EBTerm theta2;

      // partition the total rotation angle "theta"
      // check if best-fitting GB is pure twist
      theta1 = conditional(abs(axes[i] * m1) > (1.0 - _epsilon), -theta / 2.0, atan2(dir2 * m1, dirs[i] * m1)); // eta is meaningless for a twist boundary
      theta2 = conditional(abs(axes[i] * m1) > (1.0 - _epsilon), theta / 2.0, atan2(dir2 * m2, dirs[i] * m2));
      // Project m1 and m2 into the plane normal to ax and determine
      // the rotation angles of them relative to dir

      // Reduce both angles to interval (-period/2,period/2)
      // semi-open with a small numerical error
      theta2 = theta2 % period;
      theta1 = theta1 % period;

      theta2 = conditional(theta2 > period / 2, theta2 - period, theta2);
      theta1 = conditional(theta1 < period / 2, theta1 - period, theta1);

      // implement the semi-open interval in order to avoid an annoying
      // numerical problem where certain representations are double-counted
      theta2 = conditional(abs(theta2 + period / 2) < _epsilon, theta2 + period, theta2);
      theta1 = conditional(abs(theta1 + period / 2) < _epsilon, theta1 + period, theta1);

      /* Since this is only being run on fcc, which are
      centrosymmetric, and all dir vectors are 2-fold axes, then
      the operations of swapping theta1 and theta2, and of
      multilying both by -1, are symmetries for the energy
      function. This lets us fold everything into a small right
      triangle in (ksi,eta) space: */
      EBTerm ksi = abs(theta2 - theta1);
      EBTerm eta = abs(theta2 + theta1);

      // round everything to 1e-6, so that negligible numerical differences are dropped
      geom[0][thisindex] = _epsilon * (dis / _epsilon);
      geom[1][thisindex] = _epsilon * (ksi / _epsilon);
      geom[2][thisindex] = _epsilon * (eta / _epsilon);
      geom[3][thisindex] = _epsilon * (phi / _epsilon);
      geom[4][thisindex] = conditional(dis < dismax,conditional(l >= _epsilon,EBTerm(1),EBTerm(0)),EBTerm(0));
      thisindex = thisindex + 1;
    }
  }
  trimGeom(geom);
}

void
EBGBAnisoEnergy::trimGeom(std::vector<std::vector<EBTerm>> & geom)
{
  unsigned int geom_size = geom[0].size();

  if (geom_size == 0)
    return;

  // remove duplicate values. Real-counting in the same representation of one boundary messes up the
  // weighing functions in weightedMeanEnergy()
  for (unsigned int i = 0; i < (geom_size - 1); ++i)
    for (unsigned int j = i + 1; j < geom_size; ++j)
      geom[5][j] = conditional(geom[0][i] == geom[0][j] && geom[1][i] == geom[1][j] && geom[2][i] == geom[2][j] &&
          geom[3][i] == geom[3][j],0,1);

  // remove excess zeroes
  for (unsigned int i = 0; i < geom_size; ++i)
    geom[5][i] = conditional(geom[0][i] == 0 && geom[1][i] == 0 && geom[2][i] == 0 && geom[3][i] == 0,0,1);
}

ExpressionBuilder::EBTerm
EBGBAnisoEnergy::weightedMeanEnergy()
{
  // Pull out the parameters relevant to the weighting of the 100, 110, and 111 sets
  const EBTerm & eRGB(_par_vec[0]);      // The only dimensioned parameter. The energy scale,
                                        // set by the energy of a random boundary.
  const EBTerm & d0100(_par_vec[1]);     // Maximum distance for the 100 set. Also the distance
                                        // scale for the RSW weighting function.
  const EBTerm & d0110(_par_vec[2]);     // same for the 110 set
  const EBTerm & d0111(_par_vec[3]);     // same for the 111 set
  const EBTerm & weight100(_par_vec[4]); // weight for the 100 set, relative to the random boundary
  const EBTerm & weight110(_par_vec[5]); // same for 110
  const EBTerm & weight111(_par_vec[6]); // same for 111

  // Cutoff of weighting function at small d for numerical purposes
  Real offset = _epsilon;

  const unsigned int geom100_size = _geom100[0].size();
  const unsigned int geom110_size = _geom110[0].size();
  const unsigned int geom111_size = _geom111[0].size();

  mooseAssert(geom100_size == _geom100[1].size(), "geom100 size inconsistency");
  mooseAssert(geom110_size == _geom110[1].size(), "geom110 size inconsistency");
  mooseAssert(geom111_size == _geom111[1].size(), "geom111 size inconsistency");

  // Calculate energy lists in units of eRGB
  _e100.resize(geom100_size);
  set100();
  _e110.resize(geom110_size);
  set110();
  _e111.resize(geom111_size);
  set111();

  _s100.resize(geom100_size);
  _s110.resize(geom110_size);
  _s111.resize(geom111_size);

  // Calculate the weights, in a manner designed to give an RSW-like function of distance
  // Note it calculates a weight for every representation of the boundary in each set
  for (unsigned int i = 0; i < geom100_size; ++i)
    _s100[i] = conditional(_geom100[0][i] > d0100, 1.0, conditional(_geom100[0][i] < (offset * d0100), offset * libMesh::pi / 2.0, sin((libMesh::pi / 2.0) * _geom100[0][i] / d0100)));
  for (unsigned int i = 0; i < geom110_size; ++i)
    _s110[i] = conditional(_geom110[0][i] > d0110, 1.0, conditional(_geom110[0][i] < (offset * d0110), offset * libMesh::pi / 2.0, sin((libMesh::pi / 2.0) * _geom110[0][i] / d0110)));
  for (unsigned int i = 0; i < geom111_size; ++i)
    _s111[i] = conditional(_geom111[0][i] > d0111, 1.0, conditional(_geom111[0][i] < (offset * d0111), offset * libMesh::pi / 2.0, sin((libMesh::pi / 2.0) * _geom111[0][i] / d0111)));

  EBTerm w100;
  EBTerm w110;
  EBTerm w111;

  EBTerm sum100(0.0);
  EBTerm sum110(0.0);
  EBTerm sum111(0.0);

  EBTerm sumw100(0.0);
  EBTerm sumw110(0.0);
  EBTerm sumw111(0.0);

  // calculate weights and sum up results
  for (unsigned int i = 0; i < geom100_size; ++i)
  {
    w100 = (1.0 / (_s100[i] * (1.0 - 0.5 * log(_s100[i]))) - 1.0) * weight100;
    sum100 += (_e100[i] * w100);
    sumw100 += w100;
  }
  for (unsigned int i = 0; i < geom110_size; ++i)
  {
    w110 = (1 / (_s110[i] * (1.0 - 0.5 * log(_s110[i]))) - 1.0) * weight110;
    sum110 += (_e110[i] * w110);
    sumw110 += w110;
  }
  for (unsigned int i = 0; i < geom111_size; ++i)
  {
    w111 = (1.0 / (_s111[i] * (1.0 - 0.5 * log(_s111[i]))) - 1) * weight111;
    sum111 += (_e111[i] * w111);
    sumw111 += w111;
  }

  // calculate energy
  return eRGB * (sum100 + sum110 + sum111 + 1) / (sumw100 + sumw110 + sumw111 + 1);
}

void
EBGBAnisoEnergy::set100()
{
  const Real & pwr1 = _par_vec[7]; // 100 tilt/twist mix power law: twist
  const Real & pwr2 = _par_vec[8]; // 100 tilt/twist mix power law: tilt

  const unsigned int geom100_size = _geom100[1].size();
  _entwist.resize(geom100_size);
  _entilt.resize(geom100_size);

  twist100(_geom100[1]);
  aTGB100(_geom100[2], _geom100[1]);

  for (unsigned int i = 0; i < geom100_size; ++i)
    _e100[i] = _entwist[i] * pow((1.0 - (2.0 * _geom100[3][i] / libMesh::pi)), pwr1) +
               _entilt[i] * pow((2.0 * _geom100[3][i] / libMesh::pi), pwr2);
}

void
EBGBAnisoEnergy::set110()
{
  const Real & pwr1 = _par_vec[19]; // 110 tilt/twist mix power law: twist
  const Real & pwr2 = _par_vec[20]; // 110 tilt/twist mix power law: tilt

  const unsigned int geom110_size = _geom110[1].size();
  _entwist.resize(geom110_size);
  _entilt.resize(geom110_size);

  twist110(_geom110[1]);
  aTGB110(_geom110[2], _geom110[1]);

  for (unsigned int i = 0; i < geom110_size; ++i)
    _e110[i] = _entwist[i] * pow((1.0 - (2.0 * _geom110[3][i] / libMesh::pi)), pwr1) +
               _entilt[i] * pow((2.0 * _geom110[3][i] / libMesh::pi), pwr2);
}

void
EBGBAnisoEnergy::set111()
{
  const Real & a = _par_vec[34]; // linear part of 111 tilt/twist interpolation
  Real b = a - 1.0;              // ensures correct value at x = 1

  const unsigned int geom111_size = _geom111[1].size();
  _entwist.resize(geom111_size);
  _entilt.resize(geom111_size);

  twist111(_geom111[1]);
  aTGB111(_geom111[2], _geom111[1]);

  // This one fit well enough with a simple one-parameter parabola that the more
  // complicated power laws in the other sets weren't needed
  for (unsigned int i = 0; i < geom111_size; ++i)
    _e111[i] = _entwist[i] + (_entilt[i] - _entwist[i]) *
                                 (a * 2.0 * _geom111[3][i] / libMesh::pi -
                                  b * pow((2.0 * _geom111[3][i] / libMesh::pi), 2.0));
}

void
EBGBAnisoEnergy::twist100(std::vector<EBTerm> ksi)
{
  const Real Emax = _par_vec[9];         // 100 twist maximum energy
  const Real a = _par_vec[10];           // 100 twist RSW shape factor.
  const Real period = libMesh::pi / 2.0; // twist period

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    ksi[i] = abs(ksi[i]) % period; // rotation symmetry
    ksi[i] = conditional(ksi[i] > period / 2, period - ksi[i], ksi[i]);

    // implement an RSW function of ksi
    EBTerm xlogx = conditional(ksi[i] == 0, 0, sin(2 * ksi[i]) * log(sin(2 * ksi[i])));

    _entwist[i] = Emax * (sin(2 * ksi[i]) - a * xlogx);
  }
}

void
EBGBAnisoEnergy::twist110(std::vector<EBTerm> ksi)
{
  const Real th1 = _par_vec[21]; // 110 twist peak position

  const Real en1 = _par_vec[22]; // 110 twist energy peak value
  const Real en2 = _par_vec[23]; // Sigma3 energy (110 twist, so not a coherent twin)
  const Real en3 = _par_vec[24]; // energy at the symmetry point

  const Real a01 = 0.5;
  const Real a12 = 0.5;
  const Real a23 = 0.5;

  const Real th2 = acos(1.0 / 3.0); // Sigma3
  const Real th3 = libMesh::pi / 2.0; // 110 90-degree boundary is semi-special, although not a CSL
  const Real period = libMesh::pi;    // the twist period

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    ksi[i] = abs(ksi[i]) % period; // rotation symmetry
    ksi[i] = conditional(ksi[i] > period / 2.0, period - ksi[i], ksi[i]);
    _entwist[i] = conditional(ksi[i] <= th1, en1 * rSW(ksi[i], 0, th1, a01), conditional(ksi[i] > th1 && ksi[i] <= th2, en2 + (en1 - en2) * rSW(ksi[i], th2, th1, a12), en3 + (en2 - en3) * rSW(ksi[i], th3, th2, a23)));
  }
}

void
EBGBAnisoEnergy::twist111(std::vector<EBTerm> ksi)
{
  const Real thd = _par_vec[36]; // 111 twist peak position

  const Real enm = _par_vec[37]; // 111 twist energy at the peak
  const Real en2 = _par_vec[27]; // Coherent Sigma3 twin shows up in two distinct places in the code

  const Real a1 = _par_vec[35]; // 111 twist RSW shape parameter
  const Real a2 = a1;

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    ksi[i] = conditional(ksi[i] > libMesh::pi / 3.0, 2.0 * libMesh::pi / 3.0 - ksi[i], ksi[i]);
    _entwist[i] = conditional(ksi[i] <= thd, enm * rSW(ksi[i], 0, thd, a1), en2 + (enm - en2) * rSW(ksi[i], libMesh::pi / 3.0, thd, a2));
  }
}

void
EBGBAnisoEnergy::aTGB100(const std::vector<EBTerm> & eta, const std::vector<EBTerm> & ksi)
{
  const Real pwr = _par_vec[11]; // 100 aTGB interpolation power law
  const Real period = libMesh::pi / 2.0;

  _en1.resize(ksi.size());
  _en2.resize(ksi.size());
  _ksi_back.resize(ksi.size());

  sTGB100(ksi, _en1); // value at eta = 0

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
    _ksi_back[i] = period - ksi[i];
  sTGB100(_ksi_back, _en2); // value at eta = pi/2

  // eta dependence is a power law that goes from the higher to the lower,
  // whichever direction that is
  for (unsigned int i = 0; i < ksi_size; ++i)
    _entilt[i] = conditional(_en1[i] >= _en2[i], _en1[i] - (_en1[i] - _en2[i]) * pow((eta[i] / period), pwr), _en2[i] - (_en2[i] - _en1[i]) * pow((1.0 - (eta[i] / period)), pwr));
}

void
EBGBAnisoEnergy::aTGB110(const std::vector<EBTerm> & eta, const std::vector<EBTerm> & ksi)
{
  // 110 aTGB interpolation RSW shape factor
  const Real a = _par_vec[25];
  const Real period = libMesh::pi;

  const unsigned int ksi_size = ksi.size();
  _en1.resize(ksi.size());
  _en2.resize(ksi.size());
  _ksi_back.resize(ksi.size());

  sTGB110(ksi, _en1);
  for (unsigned int i = 0; i < ksi_size; ++i)
    _ksi_back[i] = period - ksi[i];
  sTGB110(_ksi_back, _en2);

  // Power-law interpolation did not work well in this case. Did an RSW function instead
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    _entilt[i] = conditional(_en1[i] >= _en2[i], _en2[i] + (_en1[i] - _en2[i]) * rSW(eta[i], libMesh::pi, 0.0, a), _en1[i] + (_en2[i] - _en1[i]) * rSW(eta[i], 0.0, libMesh::pi, a));
  }
}

void
EBGBAnisoEnergy::aTGB111(const std::vector<EBTerm> & eta, const std::vector<EBTerm> & ksi)
{
  /**
   * Below the following value of ksi, we ignore the eta dependence.  This is
   * because there's little evidence that it actually varies.  Above this
   * value, we interpolate on an RSW function that follows the Sigma3 line,
   * which is also a line of symmetry for the function.
   */
  const Real & ksim = _par_vec[38];  // 111 aTGB ksi break
  const Real & enmax = _par_vec[39]; // energy at the peak (ksi == ksim)
  const Real & enmin = _par_vec[40]; // energy at the minimum (Sigma3, eta == 0)
  const Real & encnt = _par_vec[41]; // energy at the symmetry point (Sigma3, eta == pi/3)

  Real a1 = 0.5;
  Real a2 = 0.5;

  /**
   * eta scaling parameter for 111 aTGB RSW function on Sigma3 line
   * This RSW function is unusual in that the change in shape of the
   * function is much better captured by changing the angular scale rather
   * than changing the dimensionless shape factor.
   */
  const Real & etascale = _par_vec[42];

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    const EBTerm ksii = conditional(ksi[i] > libMesh::pi / 3.0, 2.0 * libMesh::pi / 3.0 - ksi[i], ksi[i]);
    const EBTerm etai = conditional(eta[i] > libMesh::pi / 3.0, 2.0 * libMesh::pi / 3.0 - eta[i], eta[i]);

    const EBTerm chi = enmin + (encnt - enmin) * rSW(etai, 0, libMesh::pi / (2.0 * etascale), 0.5);
    _entilt[i] = conditional(ksii <= ksim, enmax * rSW(ksii, 0, ksim, a1), chi + (enmax - chi) * rSW(ksii, libMesh::pi / 3.0, ksim, a2));
  }
}

void
EBGBAnisoEnergy::sTGB100(const std::vector<EBTerm> & ksi, std::vector<EBTerm> & en)
{
  const Real & en2 = _par_vec[12]; // peak before first Sigma5
  const Real & en3 = _par_vec[13]; // first Sigma5
  const Real & en4 = _par_vec[14]; // peak between Sigma5's
  const Real & en5 = _par_vec[15]; // second Sigma5
  const Real & en6 = _par_vec[16]; // Sigma17

  const Real & th2 = _par_vec[17];                 // position of peak before first Sigma5
  const Real & th4 = _par_vec[18];                 // position of peak between Sigma5's
  Real th6 = 2 * std::acos(5.0 / std::sqrt(34.0)); // Sigma17 rotation angle

  Real a12 = 0.5; // RSW shape factor. In previous versions, these were allowed to
  Real a23 = a12; // vary, however there were too few vicinal boundaries in the
  Real a34 = a12; // ensemble to constrain them. We found that forcing the great
  Real a45 = a12; // majority of them to be 0.5 helped to constrain the fit and
  Real a56 = a12; // produced reasonable results. This holds true for most of the
  Real a67 = a12; // RSW shape factors throughout this code.

  Real en1 = 0.0; // Sigma1 at left end
  Real en7 = 0.0; // Sigma1 at right end

  Real th1 = 0.0;               // Sigma1 at left end
  Real th3 = std::acos(0.8);    // first Sigma5
  Real th5 = std::acos(0.6);    // second Sigma5
  Real th7 = libMesh::pi / 2.0; // Sigma1 at right end

  // piecewise RSW function
  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    en[i] = conditional(ksi[i] <= th2, en1 + (en2 - en1) * rSW(ksi[i], th1, th2, a12),
            conditional(ksi[i] > th2 && ksi[i] <= th3, en3 + (en2 - en3) * rSW(ksi[i], th3, th2, a23),
            conditional(ksi[i] > th3 && ksi[i] <= th4, en3 + (en4 - en3) * rSW(ksi[i], th3, th4, a34),
            conditional(ksi[i] > th4 && ksi[i] <= th5, en5 + (en4 - en5) * rSW(ksi[i], th5, th4, a45),
            conditional(ksi[i] > th5 && ksi[i] <= th6, en6 + (en5 - en6) * rSW(ksi[i], th6, th5, a56),
                                                       en7 + (en6 - en7) * rSW(ksi[i], th7, th6, a67))))));
  }
}

void
EBGBAnisoEnergy::sTGB110(const std::vector<EBTerm> & ksi, std::vector<EBTerm> & en)
{
  const Real & en2 = _par_vec[26]; // peak between Sigma1 and Sigma3
  const Real & en3 = _par_vec[27]; // Coherent Sigma3 twin relative energy; one of the more
                                   // important element-dependent parameters
  const Real & en4 = _par_vec[28]; // energy peak between Sigma3 and Sigma11
  const Real & en5 = _par_vec[29]; // Sigma11 energy
  const Real & en6 = _par_vec[30]; // energy peak between Sigma11 and Sigma1

  const Real & th2 = _par_vec[31]; // peak between Sigma1 and Sigma3
  const Real & th4 = _par_vec[32]; // peak between Sigma3 and Sigma11
  const Real & th6 = _par_vec[33]; // peak between Sigma11 and higher Sigma1

  Real a12 = 0.5;
  Real a23 = a12;
  Real a34 = a12;
  Real a45 = a12;
  Real a56 = a12;
  Real a67 = a12;

  Real en1 = 0.0;
  Real en7 = 0.0;

  Real th1 = 0.0;
  Real th3 = std::acos(1.0 / 3.0);   // Sigma3
  Real th5 = std::acos(-7.0 / 11.0); // Sigma11
  Real th7 = libMesh::pi;

  const unsigned int ksi_size = ksi.size();
  for (unsigned int i = 0; i < ksi_size; ++i)
  {
    EBTerm ksii = libMesh::pi - ksi[i]; // This is a legacy of an earlier ksi-eta mapping.
    en[i] = conditional(ksii <= th2, en1 + (en2 - en1) * rSW(ksii, th1, th2, a12),
            conditional(ksii > th2 && ksii <= th3, en3 + (en2 - en3) * rSW(ksii, th3, th2, a23),
            conditional(ksii > th3 && ksii <= th4, en3 + (en4 - en3) * rSW(ksii, th3, th4, a34),
            conditional(ksii > th4 && ksii <= th5, en5 + (en4 - en5) * rSW(ksii, th5, th4, a45),
            conditional(ksii > th5 && ksii <= th6, en5 + (en6 - en5) * rSW(ksii, th5, th6, a56),
                                                   en7 + (en6 - en7) * rSW(ksii, th7, th6, a67))))));
  }
}

ExpressionBuilder::EBTerm
EBGBAnisoEnergy::rSW(EBTerm theta, Real thetaMin, Real thetaMax, Real a)
{
  // Interval of angles where defined
  const Real dtheta = thetaMax - thetaMin;

  // Normalized angle
  const EBTerm sintheta = sin((theta - thetaMin) * libMesh::pi / (dtheta * 2));

  // Cut off a small sins to avoid 0 * infinity problem.
  // The proper limit is 0.
  return conditional(sintheta >= _epsilon, sintheta - a * (sintheta * log(sintheta)), sintheta);
}
