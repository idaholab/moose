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

#include "AddUserObjectAction.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddUserObjectAction>()
{
  return validParams<MooseObjectAction>();
}

AddUserObjectAction::AddUserObjectAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddUserObjectAction::act()
{

  // Checking to make sure that both BoundaryName and SubdomainName aren't supplied for NodalUserObjects
  if (_moose_object_pars.have_parameter<std::vector<BoundaryName> >("boundary") &&
      _moose_object_pars.have_parameter<std::vector<SubdomainName> >("block"))
  {
    const std::vector<BoundaryName> bnd_ids = _moose_object_pars.get<std::vector<BoundaryName> >("boundary");
    const std::vector<SubdomainName> blk_ids = _moose_object_pars.get<std::vector<SubdomainName> >("block");

    if (std::find(bnd_ids.begin(), bnd_ids.end(), "ANY_BOUNDARY_ID") != bnd_ids.end()
        && std::find(blk_ids.begin(), blk_ids.end(), "ANY_BLOCK_ID") != blk_ids.end())
      mooseError (std::string("The parameter 'boundary' and 'block' were both supplied for ") + getShortName());
    else if (std::find(blk_ids.begin(), blk_ids.end(), "ANY_BLOCK_ID") != blk_ids.end())
      _moose_object_pars.addPrivateParam<bool>("block_restricted_nodal", true);
  }

  _problem->addUserObject(_type, getShortName(), _moose_object_pars);
}
