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
#include "BoundaryIntegrationFunctor.h"

class FileMeshWCNSFVComponent;

/**
 * Base class for flow junctions
 */
class FileMeshWCNSFVFlowJunction : public FileMeshComponentJunction
{
public:
  FileMeshWCNSFVFlowJunction(const InputParameters & params);
  static InputParameters validParams();

protected:
  virtual void setupMesh() override;
  virtual void init() override;
  virtual void check() const override;
  virtual void addMooseObjects() override;

  /// Name of junction user object name, if any
  const std::string _junction_uo_name;
  /// How to make the components match
  const MultiMooseEnum _junction_techniques;

private:
  // Connect the variables on both sides with the same type of boundary condition
  void connectVariableWithBoundaryConditions(const PhysicsName & physics_name,
                                             const VariableName & var_name,
                                             const std::string & bc_type,
                                             bool add_first_bc);

  /// Get the connected component for WCNSFV flow
  const FileMeshWCNSFVComponent & getConnectedComponent(unsigned int connection_index) const;

  /// Get the connection functor for WCNSFV flow
  const MooseFunctorName getConnectionFunctorName(unsigned int connection_index,
                                                  const PhysicsName & physics_name,
                                                  const VariableName & var_name,
                                                  bool from_primary_side) const;

  /// Add boundary functors for connecting WCNSFV flow
  void addComponentConnectionFunctors(const PhysicsName & physics_name,
                                      const VariableName & default_var,
                                      const bool from_primary_side);

  /// Somewhere to store the connection functors. We choose here as FunctorMaterialProperty are meant for
  /// block restriction more than boundary restriction.
  std::vector<std::unique_ptr<BoundaryIntegralFunctor<Real>>> _boundary_real_functors;
  std::vector<std::unique_ptr<BoundaryIntegralFunctor<ADReal>>> _boundary_adreal_functors;
};
