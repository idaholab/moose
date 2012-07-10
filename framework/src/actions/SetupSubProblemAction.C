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

#include "SetupSubProblemAction.h"
#include "Moose.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "Conversion.h"

template<>
InputParameters validParams<SetupSubProblemAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::vector<SubdomainName> >("block", "Block IDs for the coordinate systems");
  params.addParam<std::vector<std::string> >("coord_type", "Type of the coordinate system per block param");

  params.addParam<bool>("fe_cache", false, "Whether or not to turn on the finite element shape function caching system.  This can increase speed with an associated memory cost.");
  return params;
}

SetupSubProblemAction::SetupSubProblemAction(const std::string & name, InputParameters parameters) :
    Action(name, parameters),
    _blocks(getParam<std::vector<SubdomainName> >("block")),
    _coord_sys(getParam<std::vector<std::string> >("coord_type")),
    _fe_cache(getParam<bool>("fe_cache"))
{
}

SetupSubProblemAction::~SetupSubProblemAction()
{
}

void
SetupSubProblemAction::act()
{
  if (_problem != NULL)
  {
    _problem->setCoordSystem(_blocks, _coord_sys);
    _problem->useFECache(_fe_cache);
  }
}
