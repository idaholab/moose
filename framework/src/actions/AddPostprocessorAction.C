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

#include "AddPostprocessorAction.h"
#include "Factory.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddPostprocessorAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddPostprocessorAction::AddPostprocessorAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddPostprocessorAction::act()
{
  // Do some error checking to make sure that both BoundaryName and SubdomainName aren't supplied
  // for NodalPPS.
  if (_moose_object_pars.have_parameter<std::vector<BoundaryName> >("boundary") &&
      _moose_object_pars.have_parameter<std::vector<SubdomainName> >("block"))
  {
    const std::vector<BoundaryName> bnd_ids = _moose_object_pars.get<std::vector<BoundaryName> >("boundary");
    const std::vector<SubdomainName> block_ids = _moose_object_pars.get<std::vector<SubdomainName> >("block");

    if (bnd_ids[0] != "ANY_BOUNDARY_ID" && block_ids[0] != "ANY_BLOCK_ID")
      mooseError (std::string("The parameter 'boundary' and 'block' were both supplied for ") + getShortName());
    else if (block_ids[0] != "ANY_BLOCK_ID")
      _moose_object_pars.addPrivateParam<bool>("block_restricted_nodal", true);
  }

  _problem->addPostprocessor(_type, getShortName(), _moose_object_pars);
}
