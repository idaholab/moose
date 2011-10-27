#include "Parser.h"

namespace Elk
{

void associateSyntax()
{
  /**
   * Note: the optional third parameter is used to differentiate which action_name is
   * satisfied based on the syntax encountered for classes which are registered
   * to satisfy more than one action_name
   */
  Moose::syntax.registerActionSyntax("EmptyAction", "BCs/PlenumPressure");
  Moose::syntax.registerActionSyntax("PlenumPressureAction", "BCs/PlenumPressure/*");

  Moose::syntax.registerActionSyntax("EmptyAction", "BCs/PlenumPressureRZ");
  Moose::syntax.registerActionSyntax("PlenumPressureRZAction", "BCs/PlenumPressureRZ/*");

  Moose::syntax.registerActionSyntax("EmptyAction", "BCs/Pressure");
  Moose::syntax.registerActionSyntax("PressureAction", "BCs/Pressure/*");

  Moose::syntax.registerActionSyntax("EmptyAction", "BCs/PressureRZ");
  Moose::syntax.registerActionSyntax("PressureRZAction", "BCs/PressureRZ/*");

  Moose::syntax.registerActionSyntax("ContactAction", "Contact/*");
  Moose::syntax.registerActionSyntax("ThermalContactAction", "ThermalContact/*");

  Moose::syntax.registerActionSyntax("SolidMechanicsAction", "SolidMechanics/*");
}


} // namespace
