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
#include "InitialConditionWarehouse.h"
#include "InitialCondition.h"
#include "ScalarInitialCondition.h"

InitialConditionWarehouse::InitialConditionWarehouse() :
    Warehouse<InitialCondition>()
{
}

InitialConditionWarehouse::~InitialConditionWarehouse()
{
}

void
InitialConditionWarehouse::initialSetup()
{
  // Sort the ICs
  for (std::map<SubdomainID, std::vector<InitialCondition *> >::iterator it = _all_ics.begin(); it != _all_ics.end(); ++it)
  {
    sortICs(it->second);
    for (std::vector<InitialCondition *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      (*jt)->initialSetup();
  }

  for (std::map<BoundaryID, std::vector<InitialCondition *> >::iterator it = _active_boundary_ics.begin(); it != _active_boundary_ics.end(); ++it)
  {
    sortICs(it->second);
    for (std::vector<InitialCondition *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      (*jt)->initialSetup();
  }

  sortScalarICs(_active_scalar_ics);
}

void
InitialConditionWarehouse::updateActiveICs(SubdomainID subdomain)
{
  _active_ics.clear();
  // add global ICs
  _active_ics.insert(_active_ics.end(), _all_ics[Moose::ANY_BLOCK_ID].begin(), _all_ics[Moose::ANY_BLOCK_ID].end());
  // and then block-restricted ICs
  _active_ics.insert(_active_ics.end(), _all_ics[subdomain].begin(), _all_ics[subdomain].end());
}

const std::vector<InitialCondition *> &
InitialConditionWarehouse::active() const
{
  return _active_ics;
}

bool
InitialConditionWarehouse::hasActiveBoundaryICs(BoundaryID boundary_id) const
{
  return _active_boundary_ics.find(boundary_id) != _active_boundary_ics.end();
}

const std::vector<InitialCondition *> &
InitialConditionWarehouse::activeBoundary(BoundaryID boundary_id) const
{
  std::map<BoundaryID, std::vector<InitialCondition *> >::const_iterator it = _active_boundary_ics.find(boundary_id);

  if (it == _active_boundary_ics.end())
    mooseError("No active boundary ICs on boudnary: " << boundary_id);

  return it->second;
}

void
InitialConditionWarehouse::addInitialCondition(const std::string & var_name, SubdomainID blockid, MooseSharedPointer<InitialCondition> ic)
{
  std::string name;

  if (_ics[var_name].find(blockid) != _ics[var_name].end())                     // Two ics on the same block
    name = _ics[var_name][blockid]->name();
  else if (_ics[var_name].find(Moose::ANY_BLOCK_ID) != _ics[var_name].end())    // Two ics, first global
    name = _ics[var_name][Moose::ANY_BLOCK_ID]->name();
  else if (blockid == Moose::ANY_BLOCK_ID && _ics[var_name].size())             // Two ics, second global
    name = _ics[var_name].begin()->second->name();

  if (name != "")
    mooseError(std::string("Initial Conditions '") + name + "' and '" + ic->name() + "' are both defined on the same block.");

  _ics[var_name][blockid] = ic;
  _all_ics[blockid].push_back(ic.get());
  _all_objects.push_back(ic.get());
}

void
InitialConditionWarehouse::addBoundaryInitialCondition(const std::string & var_name, BoundaryID boundary_id, MooseSharedPointer<InitialCondition> ic)
{
  if (_boundary_ics[var_name].find(boundary_id) == _boundary_ics[var_name].end())                     // Two ics on the same boundary
  {
    _boundary_ics[var_name][boundary_id] = ic;
    _active_boundary_ics[boundary_id].push_back(ic.get());
    _all_objects.push_back(ic.get());
  }
  else
    mooseError("Initial condition '" << _boundary_ics[var_name][boundary_id]->name() << "' and '" << ic->name() << "' are both defined on the same block.");
}

const std::vector<ScalarInitialCondition *> &
InitialConditionWarehouse::activeScalar() const
{
  return _active_scalar_ics;
}

void
InitialConditionWarehouse::addScalarInitialCondition(const std::string & var_name, MooseSharedPointer<ScalarInitialCondition> ic)
{
  std::map<std::string, MooseSharedPointer<ScalarInitialCondition> >::iterator it = _scalar_ics.find(var_name);
  if (it == _scalar_ics.end())
  {
    _scalar_ics[var_name] = ic;
    _active_scalar_ics.push_back(ic.get());
  }
  else
    mooseError("Initial condition for variable '" << var_name << "' has been already set.");
}


void
InitialConditionWarehouse::sortICs(std::vector<InitialCondition *> & ics)
{
  try
  {
    // Sort based on dependencies
    DependencyResolverInterface::sort<InitialCondition *>(ics);
  }
  catch (CyclicDependencyException<InitialCondition *> & e)
  {
    DependencyResolverInterface::cyclicDependencyError<InitialCondition *>(e, "Cyclic dependency detected in InitialCondition ordering");
  }
}

void
InitialConditionWarehouse::sortScalarICs(std::vector<ScalarInitialCondition *> & ics)
{
  try
  {
    // Sort based on dependencies
    DependencyResolverInterface::sort<ScalarInitialCondition *>(ics);
  }
  catch (CyclicDependencyException<ScalarInitialCondition *> & e)
  {
    DependencyResolverInterface::cyclicDependencyError<ScalarInitialCondition *>(e, "Cyclic dependency detected in ScalarInitialCondition ordering");
  }
}
