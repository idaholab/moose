//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileMeshComponent.h"
#include "NSFVBase.h"
#include "THMMesh.h"

/**
 * Navier-Stokes flow component
 */
class FlowComponentNS : public NSFVBase<FileMeshComponent>
{
public:
  static InputParameters validParams();

  FlowComponentNS(const InputParameters & parameters);

  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void init() override;

  virtual std::vector<SubdomainName> getBlocks() const override { return getSubdomainNames(); }
  virtual Factory & getFactory() override { return getMooseApp().getFactory(); }
  virtual FEProblemBase & getProblem() override { return getMooseApp().feProblem(); }
  virtual const MooseMesh & getMesh() const override { return constMesh(); }
  virtual void addNSNonlinearVariable(const std::string & var_type,
                                      const std::string & var_name,
                                      InputParameters & params) override
  {
    getTHMProblem().addSimVariable(true, var_type, var_name, params);
  }
  virtual void addNSAuxVariable(const std::string & var_type,
                                const std::string & var_name,
                                InputParameters & params) override
  {
    getTHMProblem().addSimVariable(false, var_type, var_name, params);
  }
  virtual void addNSInitialCondition(const std::string & type,
                                     const std::string & name,
                                     InputParameters & params) override
  {
    getTHMProblem().addSimInitialCondition(type, name, params);
  }
  virtual std::string prefix() const override { return name() + ":"; }
};
