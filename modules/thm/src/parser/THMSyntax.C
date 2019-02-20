#include "THMSyntax.h"
#include "ActionFactory.h"
#include "Syntax.h"

namespace THM
{

void
associateSyntax(Syntax & syntax)
{
  syntax.registerActionSyntax("GlobalSimParamAction", "GlobalParams");
  syntax.registerActionSyntax(
      "AddStabilizationSettingsAction", "Stabilizations/*", "THM:add_stabilization");
  syntax.registerActionSyntax("AddHeatStructureMaterialAction",
                              "HeatStructureMaterials/*",
                              "THM:add_heat_structure_material");
  syntax.registerActionSyntax("AddComponentAction", "Components/*", "THM:add_component");
  syntax.registerActionSyntax("PostprocessorAsControlAction", "Postprocessors/*");
}

void
registerActions(Syntax & syntax)
{
  registerTask("THM:global_sim_params", false);
  registerTask("THM:init_simulation", true);
  registerTask("THM:setup_mesh", true);
  registerTask("THM:build_mesh", true);
  registerTask("THM:init_components", true);
  registerTask("THM:identify_loops", true);
  registerTask("THM:add_variables", true);
  registerTask("THM:setup_output", true);
  registerTask("THM:add_component_physics", true);
  registerTask("THM:integrity_check", true);
  registerTask("THM:control_data_integrity_check", true);

  registerMooseObjectTask("THM:add_component", Component, false);
  registerMooseObjectTask("THM:add_heat_structure_material", SolidMaterialProperties, false);

  registerMooseObjectTask("THM:add_stabilization", StabilizationSettings, false);

  try
  {
    syntax.addDependency("THM:global_sim_params", "set_global_params");
    syntax.addDependency("setup_recover_file_base", "THM:global_sim_params");
    syntax.addDependency("THM:add_heat_structure_material", "add_function");
    syntax.addDependency("THM:add_component", "check_copy_nodal_vars");
    syntax.addDependency("THM:init_simulation", "THM:add_component");
    syntax.addDependency("THM:identify_loops", "THM:add_component");
    syntax.addDependency("THM:identify_loops", "add_fluid_properties");
    syntax.addDependency("THM:integrity_check", "THM:init_components");
    syntax.addDependency("THM:integrity_check", "THM:identify_loops");
    syntax.addDependency("THM:build_mesh", "THM:init_simulation");
    syntax.addDependency("add_partitioner", "THM:build_mesh");
    syntax.addDependency("THM:setup_mesh", "create_problem_complete");
    syntax.addDependency("add_fluid_properties", "THM:setup_mesh");
    syntax.addDependency("add_elemental_field_variable", "add_fluid_properties");
    syntax.addDependency("add_aux_variable", "add_fluid_properties");
    syntax.addDependency("add_variable", "add_fluid_properties");
    syntax.addDependency("THM:init_components", "THM:add_heat_structure_material");
    syntax.addDependency("THM:add_variables", "THM:init_components");
    syntax.addDependency("THM:setup_output", "add_output");
    syntax.addDependency("THM:add_component_physics", "add_material");
    syntax.addDependency("check_output", "THM:add_component_physics");
    syntax.addDependency("THM:control_data_integrity_check", "check_integrity");
    syntax.addDependency("add_user_object", "THM:add_variables");
    syntax.addDependency("add_output_aux_variables", "THM:add_component_physics");
    syntax.addDependency("add_periodic_bc", "THM:add_variables");
    syntax.addDependency("THM:add_stabilization", "add_function");
  }
  catch (CyclicDependencyException<std::string> & e)
  {
    mooseError("Cyclic Dependency Detected during addDependency() calls");
  }
}
}
