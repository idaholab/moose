//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesApp.h"
#include "libmesh/elem.h"

void
checkUnusedInputParameter(const InputParameters & p, const std::string & name, const std::string & explanation)
{
  if (p.isParamSetByUser(name))
    mooseDoOnce(mooseWarning(p.blockLocation() + " " + p.blockFullpath() + "\n" +
      " " + explanation + " the '" + name + "' parameter is un-used."));
}

void
checkTestOnlyParameter(const InputParameters & p)
{
  mooseDoOnce(mooseWarning(p.blockLocation() + " " + p.blockFullpath() + "\n" +
    + "This object is for testing purposes only, please check the user manual for further details."));
}

void
errorMessage(const InputParameters & p, const std::string & message)
{
  mooseError(p.blockLocation() + " " + p.blockFullpath() + "\n" + message);
}
