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
}

void
registerActions(Syntax & syntax)
{
  registerMooseObjectTask("THM:add_heat_structure_material", SolidMaterialProperties, false);

  syntax.addDependency("THM:add_heat_structure_material", "add_function");
}
}
