//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StochasticToolsApp.h"
#include "MooseObject.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

class CovarianceFunctionBase : public MooseObject
{
public:
  static InputParameters validParams();
  CovarianceFunctionBase(const InputParameters & parameters);

  /// Generates the Covariance Matrix given two points in the parameter space
  virtual void computeCovarianceMatrix(RealEigenMatrix & K,
                                       const RealEigenMatrix & x,
                                       const RealEigenMatrix & xp,
                                       const bool is_self_covariance) const = 0;

  /// Used for outputting Hyper-parameter settings
  virtual void
  buildHyperParamMap(std::unordered_map<std::string, Real> & map,
                     std::unordered_map<std::string, std::vector<Real>> & vec_map) const = 0;

  virtual void computedKdhyper(RealEigenMatrix & /*dKdhp*/,
                               const RealEigenMatrix & /*x*/,
                               unsigned int /*hyper_param_id*/) const {};

  virtual unsigned int getNumTunable() const { return _num_tunable; };

  virtual void buildHyperParamVec(libMesh::PetscVector<Number> & theta) const = 0;

  virtual void buildHyperParamBounds(libMesh::PetscVector<Number> & theta_l,
                                     libMesh::PetscVector<Number> & theta_u) const = 0;

  virtual void loadHyperParamVec(libMesh::PetscVector<Number> & theta) = 0;

protected:
  unsigned int _num_tunable = 0;
};
