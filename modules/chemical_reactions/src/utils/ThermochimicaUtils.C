//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermochimicaUtils.h"
#include "InputParameters.h"
#include "MooseObject.h"
#include "Action.h"

namespace ThermochimicaUtils
{

static const std::string message =
    "To use this object, you need to have the `Thermochimica` library installed. Refer to the "
    "documentation for guidance on how to enable it.";

void
addClassDescription(InputParameters & params, const std::string & desc)
{
#ifdef THERMOCHIMICA_ENABLED
  params.addClassDescription(desc);
#else
  params.addClassDescription(message + " (Original description: " + desc + ")");
#endif
}

void
checkLibraryAvailability(MooseObject & self)
{
#ifndef THERMOCHIMICA_ENABLED
  self.paramError("type", message);
#else
  libmesh_ignore(self);
#endif
}

void
checkLibraryAvailability(Action & self)
{
#ifndef THERMOCHIMICA_ENABLED
  mooseError(self.parameters().blockLocation() + ": " + message);
#else
  libmesh_ignore(self);
#endif
}

// namespace ThermochimicaUtils
}
