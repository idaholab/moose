//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshWCNSFVFlowJunction.h"
#include "FileMeshComponent.h"
#include "FileMeshWCNSFVComponent.h"
#include "THMMesh.h"
#include "WCNSFVPhysicsBase.h"
#include "NS.h"

#include "libmesh/boundary_info.h"

registerMooseObject("ThermalHydraulicsApp", FileMeshWCNSFVFlowJunction);

InputParameters
FileMeshWCNSFVFlowJunction::validParams()
{
  InputParameters params = FileMeshComponentJunction::validParams();
  params.addPrivateParam<std::string>("component_type", "flow_junction");

  // Belongs on a base class
  // There may be more than one physics to join!
  MultiMooseEnum junction_techniques("stitching boundary_values boundary_average_values");
  params.addRequiredParam<MultiMooseEnum>("junction_techniques",
                                          junction_techniques,
                                          "How to join the Physics on file mesh components");

  return params;
}

FileMeshWCNSFVFlowJunction::FileMeshWCNSFVFlowJunction(const InputParameters & params)
  : FileMeshComponentJunction(params),
    _junction_uo_name(genName(name(), "junction_uo")),
    _junction_techniques(getParam<MultiMooseEnum>("junction_techniques"))
{
}

void
FileMeshWCNSFVFlowJunction::setupMesh()
{
  FileMeshComponentJunction::setupMesh();
  if (_junction_techniques.contains("stitching"))
  {
    // The mesh contains all the components already, but the components are not stitched together
    THMMesh & comp_meshes = mesh();
    comp_meshes.buildNodeListFromSideList();
    comp_meshes.buildNodeList();

    for (const auto & conn_i : index_range(_connections))
    {
      if (conn_i == 0)
        continue;
      const auto & b1_name = _connections[0]._boundary_name;
      const auto & b2_name = _connections[conn_i]._boundary_name;
      MooseMeshUtils::stitchSidesets(comp_meshes, b1_name, b2_name, false);
    }
    // TODO boundary infos from the component should be invalidated now
  }
}

void
FileMeshWCNSFVFlowJunction::init()
{
  FileMeshComponentJunction::init();
  mooseAssert(_connections.size(), "Should have a connection");

  if (_junction_techniques.contains("boundary_average_values"))
  {
    // Create a functor to average the value of one to set the other
    // Pressure

    // Velocity

    // Temperature

    // Scalars
  }
}

void
FileMeshWCNSFVFlowJunction::check() const
{
  FileMeshComponentJunction::check();

  for (const auto & comp_name : _connected_component_names)
    checkComponentOfTypeExistsByName<FileMeshComponent>(comp_name);
  if (!_junction_techniques.contains("stitching"))
    for (const auto i : index_range(_connections))
      getConnectedComponent(i).hasBoundary(_connections[i]._boundary_name);
}

void
FileMeshWCNSFVFlowJunction::addMooseObjects()
{
  if (_junction_techniques.contains("boundary_values"))
  {
    // Add dirichlet BCs to force continuity
    // Set the outlet of one to be the inlet of the other and vice-versa
    // We cannot mix combinations of fully developed and not-fully developed BCs
    connectVariableWithBoundaryConditions("flow", NS::pressure, "FVADFunctorDirichletBC", true);
    connectVariableWithBoundaryConditions("flow", NS::velocity_x, "INSFVInletVelocityBC", false);
    if (constMesh().dimension() > 1 && getConnectedComponent(0).dimension() > 1)
      connectVariableWithBoundaryConditions("flow", NS::velocity_y, "INSFVInletVelocityBC", false);
    if (constMesh().dimension() > 2 && getConnectedComponent(0).dimension() > 2)
      connectVariableWithBoundaryConditions("flow", NS::velocity_z, "INSFVInletVelocityBC", false);
    connectVariableWithBoundaryConditions(
        "energy", NS::temperature, "FVADFunctorDirichletBC", false);
    // connectVariableWithBoundaryConditions("scalar", "scalar", "FVADFunctorDirichletBC", false);
  }
}

void
FileMeshWCNSFVFlowJunction::connectVariableWithBoundaryConditions(const PhysicsName & physics_name,
                                                                  const VariableName & var_name,
                                                                  const std::string & bc_type,
                                                                  bool add_first_bc)
{
  // The first component of the connections will be connected to every other
  const auto & base_comp = getConnectedComponent(0);

  // If the physics does not exist, we skip adding the connection
  if (base_comp.hasPhysics(physics_name))
  {
    const auto variable_name = base_comp.getPhysics(physics_name)->getFlowVariableName(var_name);
    for (const auto & conn_i : index_range(_connections))
    {
      if (conn_i == 0)
        continue;
      const auto other_comp_name = _connections[conn_i]._component_name;
      const FileMeshWCNSFVComponent & other_comp =
          getComponentByName<FileMeshWCNSFVComponent>(other_comp_name);

      // Only add a connection if both share the Physics
      if (other_comp.hasPhysics(physics_name))
      {
        const auto other_variable_name =
            other_comp.getPhysics(physics_name)->getFlowVariableName(var_name);

        InputParameters params = getMooseApp().getFactory().getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {_connections[0]._boundary_name};
        params.set<bool>("functor_defined_on_other_side") = true;

        // we cannot add dirichlet BCs on both sides, it would create an infinite recursion
        // Add boundary condition on one side on the junction
        if (add_first_bc)
        {
          params.set<NonlinearVariableName>("variable") = variable_name;
          params.set<MooseFunctorName>("functor") = other_variable_name;
          getMooseApp().feProblem().addFVBC(
              bc_type, name() + ":" + variable_name + "_" + _connections[0]._boundary_name, params);
        }
        // Add boundary condition on the other side of the junction
        else
        {
          params.set<NonlinearVariableName>("variable") = other_variable_name;
          params.set<MooseFunctorName>("functor") = variable_name;
          getMooseApp().feProblem().addFVBC(bc_type,
                                            name() + ":" + other_variable_name + "_" +
                                                _connections[0]._boundary_name,
                                            params);
        }
      }
    }
  }
}

const FileMeshWCNSFVComponent &
FileMeshWCNSFVFlowJunction::getConnectedComponent(unsigned int connection_index) const
{
  const auto comp_name = _connections[connection_index]._component_name;
  return getComponentByName<FileMeshWCNSFVComponent>(comp_name);
}
