#include "Parser.h"

namespace Elk
{

void associateSyntax(Syntax & syntax)
{
  /**
   * Note: the optional third parameter is used to differentiate which action_name is
   * satisfied based on the syntax encountered for classes which are registered
   * to satisfy more than one action_name
   */
  syntax.registerActionSyntax("EmptyAction", "BCs/PlenumPressure");
  syntax.registerActionSyntax("PlenumPressureAction", "BCs/PlenumPressure/*");

  syntax.registerActionSyntax("EmptyAction", "BCs/Pressure");
  syntax.registerActionSyntax("PressureAction", "BCs/Pressure/*");

  syntax.registerActionSyntax("ContactAction", "Contact/*");
  syntax.registerActionSyntax("ThermalContactAction", "ThermalContact/*");

  syntax.registerActionSyntax("SolidMechanicsAction", "SolidMechanics/*");
}


} // namespace
