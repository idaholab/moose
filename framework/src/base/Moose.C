#include "Moose.h"
#include "Factory.h"
// objects that can be created by MOOSE
#include "TimeDerivative.h"

#include "Steady.h"
#include "Transient.h"
#include "LooseCoupling.h"

#include "ParsedFunction.h"
#include "PiecewiseLinear.h"

namespace Moose {

static bool registered = false;

void
registerObjects()
{
  if (registered)
    return;

  registerObject(TimeDerivative);

  registerObject(Steady);
  registerObject(Transient);
  registerObject(LooseCoupling);

  registerObject(ParsedFunction);
  registerObject(PiecewiseLinear);

  registered = true;
}

} // namespace
