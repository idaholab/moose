//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosObjectAction.h"
#include "FEProblem.h"

InputParameters
KokkosObjectAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addPrivateParam<bool>("_kokkos_action", true);
  return params;
}

KokkosObjectAction::KokkosObjectAction(const InputParameters & params,
                                       const std::string object_type)
  : MooseObjectAction(params)
{
#ifndef MOOSE_KOKKOS_ENABLED
  mooseError("Attempted to add a Kokkos ",
             object_type,
             " but MOOSE was not compiled with Kokkos support.");
#else
  if (!_app.isKokkosAvailable())
    mooseError(
        "Attempted to add a Kokkos ", object_type, " but no GPU was detected in the system.");
#endif
}
