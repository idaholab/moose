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

#include "VerifyNodalUniqueID.h"
#include "SubProblem.h"

#include <algorithm>

template<>
InputParameters validParams<VerifyNodalUniqueID>()
{
  InputParameters params = validParams<NodalUserObject>();
  return params;
}

VerifyNodalUniqueID::VerifyNodalUniqueID(const std::string & name, InputParameters parameters) :
    NodalUserObject(name, parameters)
{}

// This object can't test every possible scenario.  For instance, it can't detect recycled ids
// It's only designed to make sure that all ids are unique in any given
void
VerifyNodalUniqueID::initialize()
{
  _all_ids.clear();
  _all_ids.reserve(_subproblem.mesh().getMesh().n_local_nodes());
}

void
VerifyNodalUniqueID::execute()
{
#ifdef LIBMESH_ENABLE_UNIQUE_ID
  _all_ids.push_back(_current_node->unique_id());
#else
  _all_ids.push_back(0);
#endif
}

void
VerifyNodalUniqueID::threadJoin(const UserObject &y)
{
  const VerifyNodalUniqueID & uo = static_cast<const VerifyNodalUniqueID &>(y);

  _all_ids.insert(_all_ids.end(), uo._all_ids.begin(), uo._all_ids.end());
}

void
VerifyNodalUniqueID::finalize()
{
  Parallel::sum(_all_ids);

  std::sort(_all_ids.begin(), _all_ids.end());
  std::vector<dof_id_type>::iterator it_end = std::unique(_all_ids.begin(), _all_ids.end());
  if (it_end != _all_ids.end())
    mooseError("Duplicate unique_ids found!");
}





