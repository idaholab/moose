//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationReporterBase.h"

/**
 * Mesh-based parameter optimization
 */
class ParameterMeshOptimization : public OptimizationReporterBase
{
public:
  static InputParameters validParams();
  ParameterMeshOptimization(const InputParameters & parameters);

  virtual void setInitialCondition(libMesh::PetscVector<Number> & param) override;
  virtual bool hasBounds() const override { return _upper_bounds.size() && _lower_bounds.size(); }
  virtual Real getUpperBound(dof_id_type i) const override;
  virtual Real getLowerBound(dof_id_type i) const override;
  virtual void computeGradient(libMesh::PetscVector<Number> & gradient) const override;
  virtual dof_id_type getNumParams() const override { return _ndof; }

protected:
  virtual void updateParameters(const libMesh::PetscVector<Number> & x) override;

  /**
   * Helper function to get index of the list of parameters from the dof index
   *
   * @param i The DoF index in the optimization vector
   * @return unsigned int The index of the parameter the DoF is representing
   */
  virtual unsigned int getParameterIndex(dof_id_type i) const;

  /// Parameter names
  const std::vector<ReporterValueName> & _parameter_names;
  /// Number of parameter vectors
  const unsigned int _nparams;
  /// Number of values for each parameter
  std::vector<dof_id_type> _nvalues;

  /// Parameter values declared as reporter data
  std::vector<std::vector<Real> *> _parameters;
  /// Gradient values declared as reporter data
  std::vector<std::vector<Real> *> _gradients;

  /// Bounds of the parameters
  const std::vector<Real> & _lower_bounds;
  const std::vector<Real> & _upper_bounds;

  /// Total number of parameters
  dof_id_type _ndof;
};
