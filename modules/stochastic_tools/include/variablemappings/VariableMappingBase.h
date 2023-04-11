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
#include "RestartableModelInterface.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

/**
 * This is an abstract base class for objects that provide mapping between a full-order
 * and a latent space.
 */
class VariableMappingBase : public MooseObject, public RestartableModelInterface
{
public:
  static InputParameters validParams();
  VariableMappingBase(const InputParameters & parameters);

  /**
   * Abstract function for building mapping for a given variable.
   * @param vname The name of the variable
   */
  virtual void buildMapping(const VariableName & vname) = 0;

  /**
   * Map a full-order solution vector (in DenseVector format) for a given variable into a
   * reduced-order vector (in a standard vector format)
   * @param vname The name of the variable
   * @param full_order_vector The full-order vector
   * @param reduced_order_vector The reduced-order vector to fill
   */
  virtual void map(const VariableName & vname,
                   const DenseVector<Real> & full_order_vector,
                   std::vector<Real> & reduced_order_vector) const = 0;

  /**
   * Map a full-order solution vector with a given global sample number for a given variable into a
   * reduced-order vector (in a standard vector format)
   * @param vname The name of the variable
   * @param global_sample_i The global sample index
   * @param reduced_order_vector The reduced-order vector to fill
   */
  virtual void map(const VariableName & vname,
                   const unsigned int global_sample_i,
                   std::vector<Real> & reduced_order_vector) const = 0;

  /**
   * Map a reduced-order vector (from the latent space) back to a full-order solution vector
   * (in DenseVector format) for a given variable
   * @param vname The name of the variable
   * @param reduced_order_vector The reduced-order vector
   * @param full_order_vector The full order vector to fill
   */
  virtual void inverse_map(const VariableName & vname,
                           const std::vector<Real> & reduced_order_vector,
                           DenseVector<Real> & full_order_vector) const = 0;

  /// Get the available variable names in this mapping
  virtual const std::vector<VariableName> & getVariableNames() { return _variable_names; }

protected:
  /// Storage for the names of the variables this mapping can handle
  const std::vector<VariableName> & _variable_names;
};
