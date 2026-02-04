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

#include "MFEMProblem.h"

class MFEMEigenproblem : public MFEMProblem
{
public:
  static InputParameters validParams();

  MFEMEigenproblem(const InputParameters & params);
  virtual ~MFEMEigenproblem() {}

  /**
   * Method called in AddMFEMSolverAction which will create the solver.
   */
  virtual void addMFEMSolver(const std::string & user_object_name,
                     const std::string & name,
                     InputParameters & parameters) override;


  /**
   * Override of MFEMProblem::addVariable. Sets a
   * MFEM grid function to be used in the MFEM solve.
   */
  virtual void addVariable(const std::string & var_type,
                   const std::string & var_name,
                   InputParameters & parameters) override;




};

#endif
