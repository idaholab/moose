#include "Moose.h"
#include "Factory.h"
#include "ImplicitSystem.h"
#include "PetscSupport.h"

// objects that can be created by MOOSE
#include "TimeDerivative.h"

#include "Steady.h"
#include "Transient.h"
#include "LooseCoupling.h"

#include "ParsedFunction.h"
#include "ParsedGradFunction.h"
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
  registerObject(ParsedGradFunction);
  registerObject(PiecewiseLinear);

  registered = true;
}

void
setSolverDefaults(ImplicitSystem & system)
{
#ifdef LIBMESH_HAVE_PETSC
  Moose::PetscSupport::petscSetDefaults(system);
#endif //LIBMESH_HAVE_PETSC
}

} // namespace
