//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMSyntax.h"
#include "ActionFactory.h"
#include "Syntax.h"

namespace THM
{

void
associateSyntax(Syntax & syntax)
{
  syntax.registerActionSyntax("AddHeatStructureMaterialAction",
                              "HeatStructureMaterials/*",
                              "THM:add_heat_structure_material");
  syntax.registerActionSyntax("THMSetUpMeshAction", "Components");
  syntax.registerActionSyntax("CreateTHMMeshAction", "Components");
  syntax.registerActionSyntax("CreateTHMMeshGeneratorAction", "Components");
  syntax.registerActionSyntax("THMBuildMeshAction", "Components");
  syntax.registerActionSyntax("THMAddRelationshipManagersAction", "Components");
  syntax.registerActionSyntax("AddComponentAction", "Components/*", "THM:add_component");
  syntax.registerActionSyntax("AddComponentAction", "Components/*/*", "THM:add_component");
  syntax.registerActionSyntax("AddClosuresAction", "Closures/*", "THM:add_closures");
  syntax.registerActionSyntax("THMAddControlAction", "ControlLogic/*", "add_control");
  syntax.registerActionSyntax("AddIterationCountPostprocessorsAction", "Debug");
  syntax.registerActionSyntax("PostprocessorAsControlAction", "Postprocessors/*");
  syntax.registerActionSyntax("THMDebugAction", "Debug");
  syntax.registerActionSyntax("THMPrintComponentLoopsAction", "Debug");
  syntax.registerActionSyntax("THMOutputVectorVelocityAction", "Outputs");
  syntax.registerActionSyntax("CoupledHeatTransferAction", "CoupledHeatTransfers/*", "add_bc");
  syntax.registerActionSyntax(
      "CoupledHeatTransferAction", "CoupledHeatTransfers/*", "add_user_object");
  syntax.registerActionSyntax(
      "CoupledHeatTransferAction", "CoupledHeatTransfers/*", "add_transfer");
}

void
registerActions(Syntax & syntax)
{
  registerTask("THM:init_simulation", true);
  registerTask("THM:create_thm_mesh", false);
  registerTask("THM:set_up_coordinate_system", true);
  registerTask("THM:create_thm_problem", true);
  registerTask("THM:create_thm_mesh", false);
  registerTask("THM:create_thm_mesh_generator", false);
  registerTask("THM:build_mesh", false);
  registerTask("THM:init_components", true);
  registerTask("THM:identify_loops", true);
  registerTask("THM:add_variables", true);
  registerTask("THM:setup_output", true);
  registerTask("THM:add_component_moose_objects", true);
  registerTask("THM:integrity_check", true);
  registerTask("THM:control_data_integrity_check", true);
  registerTask("THM:preconditioning_integrity_check", true);
  registerTask("THM:setup_quadrature", true);
  registerTask("THM:debug_action", false);
  registerTask("THM:print_component_loops", false);
  registerTask("THM:output_vector_velocity", true);
  registerTask("THM:add_relationship_managers", false);

  registerMooseObjectTask("THM:add_component", Component, false);
  registerMooseObjectTask("THM:add_heat_structure_material", SolidMaterialProperties, false);
  registerMooseObjectTask("THM:add_closures", Closures, false);

  try
  {
    syntax.addDependency("THM:add_heat_structure_material", "add_function");
    syntax.addDependency("THM:output_vector_velocity", "setup_mesh");
    syntax.addDependency("THM:add_closures", "setup_mesh");

    // FEProblem requires that the MeshBase pointer not be nullptr.
    syntax.addDependency("THM:create_thm_problem", "set_mesh_base");

    syntax.addDependency("THM:add_component", "THM:create_thm_mesh");
    syntax.addDependency("THM:add_component", "THM:output_vector_velocity");
    syntax.addDependency("THM:debug_action", "setup_mesh");
    syntax.addDependency("create_problem_default", "THM:create_thm_problem");
    syntax.addDependency("THM:init_simulation", "THM:add_component");
    syntax.addDependency("add_mesh_generator", "THM:add_component");
    syntax.addDependency("THM:identify_loops", "THM:add_component");
    syntax.addDependency("THM:identify_loops", "add_fluid_properties");
    syntax.addDependency("THM:integrity_check", "THM:init_components");
    syntax.addDependency("THM:integrity_check", "THM:identify_loops");
    syntax.addDependency("THM:integrity_check", "THM:debug_action");
    syntax.addDependency("THM:build_mesh", "THM:init_simulation");
    syntax.addDependency("THM:build_mesh", "THM:create_thm_mesh");
    syntax.addDependency("add_mesh_generator", "THM:create_thm_mesh");
    syntax.addDependency("execute_mesh_generators", "THM:build_mesh");
    syntax.addDependency("THM:set_up_coordinate_system", "create_problem_complete");
    syntax.addDependency("add_fluid_properties", "THM:set_up_coordinate_system");
    syntax.addDependency("add_elemental_field_variable", "add_fluid_properties");
    syntax.addDependency("add_aux_variable", "add_fluid_properties");
    syntax.addDependency("add_variable", "add_fluid_properties");
    syntax.addDependency("THM:init_components", "THM:add_heat_structure_material");
    syntax.addDependency("THM:init_components", "THM:add_closures");
    syntax.addDependency("THM:add_variables", "THM:init_components");
    syntax.addDependency("THM:setup_output", "add_output");
    syntax.addDependency("THM:add_component_moose_objects", "add_material");
    syntax.addDependency("check_output", "THM:add_component_moose_objects");
    syntax.addDependency("THM:control_data_integrity_check", "check_integrity");
    syntax.addDependency("add_user_object", "THM:add_variables");
    syntax.addDependency("add_output_aux_variables", "THM:add_component_moose_objects");
    syntax.addDependency("add_periodic_bc", "THM:add_variables");
    syntax.addDependency("THM:print_component_loops", "THM:control_data_integrity_check");
    syntax.addDependency("THM:preconditioning_integrity_check", "check_integrity");
    syntax.addDependency("THM:add_relationship_managers", "add_geometric_rm");

    // The THMMesh builds its MeshBase using the system MooseMesh.
    syntax.addDependency("THM:create_thm_mesh", "setup_mesh");

    // The THM mesh MeshBase is used in the construction of the relationship manager.
    syntax.addDependency("THM:add_relationship_managers", "THM:create_thm_mesh");

    syntax.addDependency("THM:output_vector_velocity", "THM:add_relationship_managers");
    syntax.addDependency("THM:add_variables", "THM:integrity_check");
  }
  catch (CyclicDependencyException<std::string> & e)
  {
    mooseError("Cyclic Dependency Detected during addDependency() calls");
  }
}
}
