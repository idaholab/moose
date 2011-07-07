#include "MooseSyntax.h"
#include "Parser.h"

namespace Elk
{

void associateSyntax(Parser & p)
{
  /**
   * Note: the optional third parameter is used to differentiate which action_name is
   * satisified based on the syntax encountered for classes which are registered
   * to satisfy more than one action_name
   */
  p.registerActionSyntax("EmptyAction", "BCs/PlenumPressure");
  p.registerActionSyntax("PlenumPressureAction", "BCs/PlenumPressure/*");

  p.registerActionSyntax("EmptyAction", "BCs/PlenumPressureRZ");
  p.registerActionSyntax("PlenumPressureRZAction", "BCs/PlenumPressureRZ/*");

  p.registerActionSyntax("EmptyAction", "BCs/Pressure");
  p.registerActionSyntax("PressureAction", "BCs/Pressure/*");

  p.registerActionSyntax("EmptyAction", "BCs/PressureRZ");
  p.registerActionSyntax("PressureRZAction", "BCs/PressureRZ/*");

  p.registerActionSyntax("ContactAction", "Contact/*");
  p.registerActionSyntax("ThermalContactAction", "ThermalContact/*");
}

  
} // namespace
