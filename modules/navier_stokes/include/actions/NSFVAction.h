//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// #include "Action.h"
#include "NSFVBase.h"

/**
 * This class allows us to have a section of the input file like the
 * following which automatically adds variables, kernels, aux kernels, bcs
 * for setting up the incompressible/weakly-compressible Navier-Stokes equations.
 * Create it using the following input syntax:
 * [Modules]
 *   [NavierStokesFV]
 *     param_1 = value_1
 *     param_2 = value_2
 *     ...
 *   []
 * []
 */
class NSFVAction : public NSFVBase<Action>
{
public:
  static InputParameters validParams();

  NSFVAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /**
   * Add relationship manager to extend the number of ghosted layers if necessary.
   * This is executed before the kernels and other objects are added.
   */
  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  virtual std::vector<SubdomainName> getBlocks() const override
  {
    return getParam<std::vector<SubdomainName>>("block");
  }
  virtual Factory & getFactory() override { return _factory; }
  virtual FEProblemBase & getProblem() override { return *_problem; }
  virtual const MooseMesh & getMesh() const override { return *_mesh; }
  virtual void addNSNonlinearVariable(const std::string & var_type,
                                      const std::string & var_name,
                                      InputParameters & params) override
  {
    getProblem().addVariable(var_type, var_name, params);
  }
  virtual void addNSAuxVariable(const std::string & var_type,
                                const std::string & var_name,
                                InputParameters & params) override
  {
    getProblem().addAuxVariable(var_type, var_name, params);
  }
  virtual void addNSInitialCondition(const std::string & type,
                                     const std::string & name,
                                     InputParameters & params) override
  {
    getProblem().addInitialCondition(type, name, params);
  }
  virtual std::string prefix() const override { return ""; }
};
