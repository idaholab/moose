//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"

namespace libMesh
{
template <typename>
class VectorValue;
typedef VectorValue<Real> RealGradient;
}

/**
 * This class provides interface for extracting the forces and torques computed in other UserObjects
 */
class GrainForceAndTorqueInterface
{
public:
  virtual const std::vector<RealGradient> & getForceValues() const = 0;
  virtual const std::vector<RealGradient> & getTorqueValues() const = 0;
  virtual const std::vector<Real> & getForceCJacobians() const = 0;
  virtual const std::vector<std::vector<Real>> & getForceEtaJacobians() const = 0;
};
