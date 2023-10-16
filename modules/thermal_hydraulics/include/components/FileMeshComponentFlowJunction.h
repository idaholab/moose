//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileMeshComponentJunction.h"

/**
 * Base class for flow junctions
 */
class FileMeshComponentFlowJunction : public FileMeshComponentJunction
{
public:
  FileMeshComponentFlowJunction(const InputParameters & params);

protected:
  virtual void setupMesh() override;
  virtual void init() override;
  virtual void check() const override;
  virtual void addMooseObjects() override;

  /// Name of junction user object name, if any
  const std::string _junction_uo_name;
  /// How to make the components match
  const MooseEnum _junction_technique;

private:
  // Connect the variables on both sides with the same type of boundary condition
  void connectVariableWithBoundaryConditions(const PhysicsName & physics_name,
                                             const VariableName & var_name,
                                             const std::string & bc_type,
                                             bool add_first_bc);

public:
  static InputParameters validParams();
};
