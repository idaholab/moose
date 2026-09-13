//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMObject.h"
#include "EquationSystem.h"

class MFEMBoundaryCondition;
class MFEMKernel;

/**
 * Base class for MOOSE objects that construct objects derived from Moose::MFEM::EquationSystem.
 */
class MFEMWeakFormBase : public MFEMObject
{
public:
  static InputParameters validParams();

  MFEMWeakFormBase(const InputParameters & parameters);

  /// Constructs the EquationSystem, adds the requested kernels and boundary conditions to it, and
  /// initialises it. Derived classes supply the system itself through makeEquationSystem().
  std::shared_ptr<Moose::MFEM::EquationSystem> createEquationSystem();

protected:
  /// Constructs the empty EquationSystem of the type this weak form builds.
  virtual std::shared_ptr<Moose::MFEM::EquationSystem> makeEquationSystem() = 0;

  virtual void addBoundaryCondition(const std::string & name,
                                    std::shared_ptr<MFEMBoundaryCondition> bc) = 0;

  virtual void addKernel(const std::string & name, std::shared_ptr<MFEMKernel> kernel) = 0;

  /// Initialise the equation system. TODO: move all setup into EquationSystem constructors
  void initEquationSystem();

  std::vector<MFEMBoundaryConditionName> _bc_names;
  std::vector<MFEMKernelName> _kernel_names;

  /// Stores the constructed EquationSystem. Kept non-public so that it is reached only through
  /// createEquationSystem(), which returns it once fully initialised.
  std::shared_ptr<Moose::MFEM::EquationSystem> _equation_system{nullptr};
};

#endif
