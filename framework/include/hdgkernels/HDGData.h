//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/libmesh_config.h"
#include <Eigen/Dense>

#ifdef LIBMESH_USE_COMPLEX_NUMBERS
typedef Eigen::MatrixXcd EigenMatrix;
typedef Eigen::VectorXcd EigenVector;
#else
typedef Eigen::MatrixXd EigenMatrix;
typedef Eigen::VectorXd EigenVector;
#endif

class HDGData
{
protected:
  /**
   * Indicate what types of physics are being solved
   */
  virtual std::string physics() const = 0;

  /**
   * The primal and Lagrange multiplier variables that contribute to the HDG discretization
   */
  virtual std::set<const MooseVariableBase *> variables() const = 0;

  /// Matrix data structures for on-diagonal coupling
  EigenMatrix _PrimalMat, _LMMat;
  /// Vector data structures
  EigenVector _PrimalVec, _LMVec;
  /// Matrix data structures for off-diagonal coupling
  EigenMatrix _PrimalLM, _LMPrimal;
};
