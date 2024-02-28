//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "libmesh/ignore_warnings.h"
#include "EulerAngles.h"
#include "MooseRandom.h"

EulerAngles::EulerAngles()
{
  phi1 = 0.0;
  Phi = 0.0;
  phi2 = 0.0;
}

EulerAngles::EulerAngles(const Eigen::Quaternion<Real> & q)
{
  phi1 = std::atan2((q.x() * q.z() + q.w() * q.y()), -(-q.w() * q.x() + q.y() * q.z())) *
         (180.0 / libMesh::pi);
  Phi = std::atan2(
            std::sqrt(1 -
                      std::pow(q.w() * q.w() - q.x() * q.x() - q.y() * q.y() + q.z() * q.z(), 2.0)),
            q.w() * q.w() - q.x() * q.x() - q.y() * q.y() + q.z() * q.z()) *
        (180.0 / libMesh::pi);
  phi2 = std::atan2((q.x() * q.z() - q.w() * q.y()), (q.w() * q.x() + q.y() * q.z())) *
         (180.0 / libMesh::pi);

  // Following checks and updates are done only to comply with bunge euler angle definitions, 0.0
  // <= phi1/phi2 <= 360.0
  if (phi1 < 0.0)
    phi1 += 360.0;
  if (phi2 < 0.0)
    phi2 += 360.0;
  if (Phi < 0.0)
    mooseError("Euler angle out of range.");
}

Eigen::Quaternion<Real>
EulerAngles::toQuaternion()
{
  Eigen::Quaternion<Real> q;

  Real cPhi1PlusPhi2, cphi, cPhi1MinusPhi2;
  Real sPhi1PlusPhi2, sphi, sPhi1MinusPhi2;

  /**
   * "NASA Mission Planning and Analysis Division.
   * "Euler Angles, Quaternions, and Transformation Matrices". NASA.
   */

  cPhi1PlusPhi2 = std::cos((phi1 * libMesh::pi / 180.0 + phi2 * libMesh::pi / 180.0) / 2.0);
  cphi = std::cos(Phi * libMesh::pi / 360.0);
  cPhi1MinusPhi2 = std::cos((phi1 * libMesh::pi / 180.0 - phi2 * libMesh::pi / 180.0) / 2.0);

  sPhi1PlusPhi2 = std::sin((phi1 * libMesh::pi / 180.0 + phi2 * libMesh::pi / 180.0) / 2.0);
  sphi = std::sin(Phi * libMesh::pi / 360.0);
  sPhi1MinusPhi2 = std::sin((phi1 * libMesh::pi / 180.0 - phi2 * libMesh::pi / 180.0) / 2.0);

  q.w() = cphi * cPhi1PlusPhi2;
  q.x() = sphi * cPhi1MinusPhi2;
  q.y() = sphi * sPhi1MinusPhi2;
  q.z() = cphi * sPhi1PlusPhi2;

  return q;
}

void
EulerAngles::random()
{
  phi1 = MooseRandom::rand() * 360.0;
  Phi = std::acos(1.0 - 2.0 * MooseRandom::rand()) / libMesh::pi * 180.0;
  phi2 = MooseRandom::rand() * 360;
}

void
EulerAngles::random(MooseRandom & random)
{
  phi1 = random.rand(0) * 360.0;
  Phi = std::acos(1.0 - 2.0 * random.rand(0)) / libMesh::pi * 180.0;
  phi2 = random.rand(0) * 360;
}
#include "libmesh/restore_warnings.h"
