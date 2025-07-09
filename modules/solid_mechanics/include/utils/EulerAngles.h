//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "libmesh/vector_value.h"
// Ignore depcrecated copy warnings for this class from libmesh Eigen contrib
#include "libmesh/ignore_warnings.h"
#include <Eigen/Geometry>

// forward declaration
class MooseRandom;

/**
 * Euler angle triplet.
 */
class EulerAngles
{
public:
  Real phi1, Phi, phi2;

  // default constructor
  EulerAngles();
  // Quaternions to Euler Angles
  EulerAngles(const Eigen::Quaternion<Real> & q);
  // Components to Euler Angles
  EulerAngles(const Real & v0, const Real & v1, const Real & v2);

  operator RealVectorValue() const { return RealVectorValue(phi1, Phi, phi2); }

  void random();
  void random(MooseRandom & random);
  // Euler to Quaternions
  Eigen::Quaternion<Real> toQuaternion();
};

// Restore warnings for other classes
#include "libmesh/restore_warnings.h"
