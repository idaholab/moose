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
  syntax.registerActionSyntax("THMCreateMeshAction", "Components");
  syntax.registerActionSyntax("AddComponentAction", "Components/*", "THM:add_component");
  syntax.registerActionSyntax("AddComponentAction", "Components/*/*", "THM:add_component");
  syntax.registerActionSyntax("AddClosuresAction", "Closures/*", "THM:add_closures");
  syntax.registerActionSyntax("THMAddControlAction", "ControlLogic/*", "THM:add_control_logic");
  syntax.registerTaskName("THM:add_control_logic", "THMControl", false);
  syntax.registerActionSyntax("AddIterationCountPostprocessorsAction", "Debug");
  syntax.registerActionSyntax("PostprocessorAsControlAction", "Postprocessors/*");
  syntax.registerActionSyntax("THMDebugAction", "Debug");
  syntax.registerActionSyntax("THMPrintComponentLoopsAction", "Debug");
  syntax.registerActionSyntax("THMOutputVectorVelocityAction", "Outputs");
  syntax.registerActionSyntax("THMSetupOutputAction", "Outputs");
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
  registerTask("THM:setup_mesh", true);
  registerTask("THM:build_mesh", true);
  registerTask("THM:init_components", true);
  registerTask("THM:identify_loops", true);
  registerTask("THM:add_variables", true);
  registerTask("THM:add_control_logic", true);
  registerTask("THM:setup_output", true);
  registerTask("THM:add_component_moose_objects", true);
  registerTask("THM:integrity_check", true);
  registerTask("THM:control_data_integrity_check", true);
  registerTask("THM:preconditioning_integrity_check", true);
  registerTask("THM:setup_quadrature", true);
  registerTask("THM:debug_action", false);
  registerTask("THM:print_component_loops", false);
  registerTask("THM:output_vector_velocity", true);
  registerTask("THM:add_relationship_managers", true);

  registerMooseObjectTask("THM:add_component", Component, false);
  registerMooseObjectTask("THM:add_heat_structure_material", SolidMaterialProperties, false);
  registerMooseObjectTask("THM:add_closures", Closures, false);

  try
  {
    syntax.addDependency("THM:output_vector_velocity", "setup_mesh");
    syntax.addDependency("THM:add_closures", "setup_mesh");
    syntax.addDependency("THM:init_components", "THM:output_vector_velocity");
    syntax.addDependency("THM:debug_action", "setup_mesh");
    syntax.addDependency("THM:init_simulation", "THM:add_component");
    syntax.addDependency("add_mesh_generator", "THM:add_component");
    syntax.addDependency("THM:identify_loops", "THM:add_component");
    // Components must specify their blocks to the Physics before it gets initialized
    syntax.addDependency("init_physics", "THM:init_components");
    // Fluid properties are retrieved during component initialization
    syntax.addDependency("THM:init_components", "add_fluid_properties");
    // Solid material property used in a component needs a function
    syntax.addDependency("THM:init_components", "add_function");
    syntax.addDependency("THM:identify_loops", "add_fluid_properties");
    syntax.addDependency("THM:integrity_check", "THM:init_components");
    syntax.addDependency("THM:integrity_check", "THM:identify_loops");
    syntax.addDependency("THM:integrity_check", "THM:debug_action");
    syntax.addDependency("THM:build_mesh", "THM:init_simulation");
    syntax.addDependency("add_mesh_generator", "THM:build_mesh");
    syntax.addDependency("THM:setup_mesh", "create_problem_complete");
    syntax.addDependency("add_fluid_properties", "THM:setup_mesh");
    syntax.addDependency("add_elemental_field_variable", "add_fluid_properties");
    syntax.addDependency("add_aux_variable", "add_fluid_properties");
    syntax.addDependency("add_variable", "add_fluid_properties");
    syntax.addDependency("THM:init_components", "THM:add_heat_structure_material");
    syntax.addDependency("THM:init_components", "THM:add_closures");
    syntax.addDependency("add_variable", "THM:init_components");
    syntax.addDependency("THM:setup_output", "add_output");
    syntax.addDependency("THM:add_component_moose_objects", "add_material");
    syntax.addDependency("check_output", "THM:add_component_moose_objects");
    syntax.addDependency("THM:add_control_logic", "add_control");
    syntax.addDependency("THM:control_data_integrity_check", "check_integrity");
    syntax.addDependency("add_user_object", "THM:add_variables");
    syntax.addDependency("add_output_aux_variables", "THM:add_component_moose_objects");
    syntax.addDependency("add_periodic_bc", "THM:add_variables");
    syntax.addDependency("THM:print_component_loops", "THM:control_data_integrity_check");
    syntax.addDependency("THM:preconditioning_integrity_check", "check_integrity");
    syntax.addDependency("THM:add_relationship_managers", "add_geometric_rm");
    syntax.addDependency("THM:add_relationship_managers", "THM:add_component");
    syntax.addDependency("THM:init_simulation", "THM:add_relationship_managers");
    syntax.addDependency("THM:output_vector_velocity", "THM:add_relationship_managers");
    syntax.addDependency("THM:add_variables", "THM:integrity_check");
  }
  catch (CyclicDependencyException<std::string> & e)
  {
    mooseError("Cyclic Dependency Detected during addDependency() calls");
  }
}
}
