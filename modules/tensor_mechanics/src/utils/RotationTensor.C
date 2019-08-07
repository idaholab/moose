//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RotationTensor.h"
#include "libmesh/libmesh.h"

RotationTensor::RotationTensor(Axis axis, Real angle) { update(axis, angle); }

RotationTensor::RotationTensor(const RealVectorValue & euler_angles) { update(euler_angles); }

void
RotationTensor::update(Axis axis, Real angle)
{
  zero();

  RealVectorValue a;
  a(axis) = 1.0;

  const Real s = std::sin(angle * libMesh::pi / 180.0);
  const Real c = std::cos(angle * libMesh::pi / 180.0);

  // assemble row wise
  _coords[0] = a * RealVectorValue(1.0, -c, -c);
  _coords[1] = a * RealVectorValue(0.0, 0.0, s);
  _coords[2] = a * RealVectorValue(0.0, -s, 0.0);

  _coords[3] = a * RealVectorValue(0.0, 0.0, -s);
  _coords[4] = a * RealVectorValue(-c, 1.0, -c);
  _coords[5] = a * RealVectorValue(s, 0.0, 0.0);

  _coords[6] = a * RealVectorValue(0.0, s, 0.0);
  _coords[7] = a * RealVectorValue(-s, 0.0, 0.0);
  _coords[8] = a * RealVectorValue(-c, -c, 1.0);
}

void
RotationTensor::update(const RealVectorValue & euler_angles)
{
  const Real phi_1 = euler_angles(0) * (libMesh::pi / 180.0);
  const Real Phi = euler_angles(1) * (libMesh::pi / 180.0);
  const Real phi_2 = euler_angles(2) * (libMesh::pi / 180.0);

  const Real c1 = std::cos(phi_1);
  const Real c2 = std::cos(Phi);
  const Real c3 = std::cos(phi_2);

  const Real s1 = std::sin(phi_1);
  const Real s2 = std::sin(Phi);
  const Real s3 = std::sin(phi_2);

  // doing a Z1, X2, Z3 rotation
  // RealTensorValue is formed row-wise

  _coords[0] = c1 * c3 - c2 * s1 * s3;  // R11
  _coords[3] = -c1 * s3 - c2 * c3 * s1; // R21
  _coords[6] = s1 * s2;                 // R31

  _coords[1] = c3 * s1 + c1 * c2 * s3; // R12
  _coords[4] = c1 * c2 * c3 - s1 * s3; // R22
  _coords[7] = -c1 * s2;               // R32

  _coords[2] = s2 * s3; // R13
  _coords[5] = c3 * s2; // R23
  _coords[8] = c2;      // R33
}
