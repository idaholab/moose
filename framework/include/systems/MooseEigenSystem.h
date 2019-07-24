//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NonlinearSystem.h"

// libMesh
#include "libmesh/eigen_system.h"

class FEProblemBase;

class MooseEigenSystem : public NonlinearSystem
{
public:
  MooseEigenSystem(FEProblemBase & problem, const std::string & name);
  virtual ~MooseEigenSystem();

  /**
   * Adds a kernel
   * @param kernel_name The type of the kernel.
   * @param name The name of the kernel.
   * @param parameters Kernel parameters.
   */
  virtual void addKernel(const std::string & kernel_name,
                         const std::string & name,
                         InputParameters & parameters);

  /**
   * Mark a variable as a variable of the eigen system
   * @param var_name The name of the variable.
   */
  virtual void markEigenVariable(const VariableName & var_name);

  /**
   * System or kernel tags
   */
  enum SYSTEMTAG
  {
    ALL,
    EIGEN
  };

  /**
   * Scale the solution vector
   *
   * @param tag System tag.
   * @param factor The scaling factor.
   */
  void scaleSystemSolution(SYSTEMTAG tag, Real scaling_factor);

  /**
   * Linear combination of the solution vectors
   *
   * @param tag System tag.
   * @param coefficients Coefficients for current, old and older solutions.
   */
  void combineSystemSolution(SYSTEMTAG tag, const std::vector<Real> & coefficients);

  /**
   * Initialize the solution vector with a constant value
   *
   * @param tag System tag.
   * @param v The value.
   */
  void initSystemSolution(SYSTEMTAG tag, Real v);
  void initSystemSolutionOld(SYSTEMTAG tag, Real v);

  /**
   * Ask eigenkernels to operate on old or current solution vectors
   */
  void eigenKernelOnOld();
  void eigenKernelOnCurrent();

  /**
   * Build DoF indices for a system
   */
  void buildSystemDoFIndices(SYSTEMTAG tag = ALL);

  /**
   * Return if eigen kernels should be on old solution
   */
  bool activeOnOld();

  /**
   * Get variable names of the eigen system
   */
  const std::set<VariableName> & getEigenVariableNames() const { return _eigen_var_names; }

  /**
   * Weather or not the system contains eigen kernels
   */
  bool containsEigenKernel() const;

protected:
  std::set<VariableName> _eigen_var_names;
  bool _all_eigen_vars;
  std::set<dof_id_type> _eigen_var_indices;

  bool _active_on_old;

  /// counter of eigen kernels
  unsigned int _eigen_kernel_counter;
};
