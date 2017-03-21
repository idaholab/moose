/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "DetermineSystemType.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<DetermineSystemType>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.mooseObjectSyntaxVisibility(false);
  return params;
}

DetermineSystemType::DetermineSystemType(InputParameters parameters) : MooseObjectAction(parameters)
{
}

void
DetermineSystemType::act()
{
  /**
   * Determine whether the Executioner is derived from EigenExecutionerBase and
   * set a flag on MooseApp that can be used during problem construction.
   */
  if (_moose_object_pars.isParamValid("_eigen") && _moose_object_pars.get<bool>("_eigen"))
    _app.useNonlinear() = false;
}
