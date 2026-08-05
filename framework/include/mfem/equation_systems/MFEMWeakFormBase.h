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

  /// Constructs the EquationSystem.
  virtual std::shared_ptr<Moose::MFEM::EquationSystem> createEquationSystem() = 0;

protected:
  virtual void addBoundaryCondition(const std::string & name,
                                    std::shared_ptr<MFEMBoundaryCondition> bc) = 0;

  virtual void addKernel(const std::string & name, std::shared_ptr<MFEMKernel> kernel) = 0;

  /// Initialise the equation system. TODO: move all setup into EquationSystem constructors
  void initEquationSystem(std::shared_ptr<Moose::MFEM::EquationSystem> equation_system);

  std::vector<MFEMBoundaryConditionName> _bc_names;
  std::vector<MFEMKernelName> _kernel_names;
};

#endif
