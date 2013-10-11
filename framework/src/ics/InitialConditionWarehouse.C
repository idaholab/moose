#include "InitialConditionWarehouse.h"
#include "InitialCondition.h"
#include "ScalarInitialCondition.h"

InitialConditionWarehouse::InitialConditionWarehouse()
{
}

InitialConditionWarehouse::~InitialConditionWarehouse()
{
  for (std::map<std::string, std::map<SubdomainID, InitialCondition *> >::iterator it = _ics.begin(); it != _ics.end(); ++it)
    for (std::map<SubdomainID, InitialCondition *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      delete jt->second;
  for (std::map<std::string, std::map<BoundaryID, InitialCondition *> >::iterator it = _boundary_ics.begin(); it != _boundary_ics.end(); ++it)
    for (std::map<BoundaryID, InitialCondition *>::iterator jt = it->second.begin(); jt != it->second.end(); ++jt)
      delete jt->second;
  for (std::map<std::string, ScalarInitialCondition *>::iterator it = _scalar_ics.begin(); it != _scalar_ics.end(); ++it)
    delete it->second;
}

void
InitialConditionWarehouse::initialSetup()
{
  // Sort the ICs
  for (std::map<SubdomainID, std::vector<InitialCondition *> >::iterator it = _all_ics.begin(); it != _all_ics.end(); ++it)
    sortICs(_all_ics[(*it).first]);
  for (std::map<BoundaryID, std::vector<InitialCondition *> >::iterator it = _active_boundary_ics.begin(); it != _active_boundary_ics.end(); ++it)
    sortICs(_active_boundary_ics[(*it).first]);

  for (std::map<std::string, std::map<SubdomainID, InitialCondition *> >::iterator it1 = _ics.begin(); it1 != _ics.end(); ++it1)
    for (std::map<SubdomainID, InitialCondition *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
      it2->second->initialSetup();
  for (std::map<std::string, std::map<BoundaryID, InitialCondition *> >::iterator it1 = _boundary_ics.begin(); it1 != _boundary_ics.end(); ++it1)
    for (std::map<BoundaryID, InitialCondition *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
      it2->second->initialSetup();

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
InitialConditionWarehouse::active()
{
  return _active_ics;
}

const std::vector<InitialCondition *> &
InitialConditionWarehouse::activeBoundary(BoundaryID boundary_id)
{
  return _active_boundary_ics[boundary_id];
}

void
InitialConditionWarehouse::addInitialCondition(const std::string & var_name, SubdomainID blockid, InitialCondition * ic)
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
  _all_ics[blockid].push_back(ic);
}

void
InitialConditionWarehouse::addBoundaryInitialCondition(const std::string & var_name, BoundaryID boundary_id, InitialCondition * ic)
{
  if (_boundary_ics[var_name].find(boundary_id) == _boundary_ics[var_name].end())                     // Two ics on the same boundary
  {
    _boundary_ics[var_name][boundary_id] = ic;
    _active_boundary_ics[boundary_id].push_back(ic);
  }
  else
    mooseError("Initial condition '" << _boundary_ics[var_name][boundary_id]->name() << "' and '" << ic->name() << "' are both defined on the same block.");
}

const std::vector<ScalarInitialCondition *> &
InitialConditionWarehouse::activeScalar()
{
  return _active_scalar_ics;
}

void
InitialConditionWarehouse::addScalarInitialCondition(const std::string & var_name, ScalarInitialCondition * ic)
{
  std::map<std::string, ScalarInitialCondition *>::iterator it = _scalar_ics.find(var_name);
  if (it == _scalar_ics.end())
  {
    _scalar_ics[var_name] = ic;
    _active_scalar_ics.push_back(ic);
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
    DependencyResolverInterface::sort(ics.begin(), ics.end());
  }
  catch (CyclicDependencyException<DependencyResolverInterface *> & e)
  {
    std::ostringstream oss;

    oss << "Cyclic dependency detected in aux kernel ordering:" << std::endl;
    const std::multimap<DependencyResolverInterface *, DependencyResolverInterface *> & depends = e.getCyclicDependencies();
    for (std::multimap<DependencyResolverInterface *, DependencyResolverInterface *>::const_iterator it = depends.begin(); it != depends.end(); ++it)
      oss << (static_cast<InitialCondition *>(it->first))->name() << " -> " << (static_cast<InitialCondition *>(it->second))->name() << std::endl;
    mooseError(oss.str());
  }
}

void
InitialConditionWarehouse::sortScalarICs(std::vector<ScalarInitialCondition *> & ics)
{
  try
  {
    // Sort based on dependencies
    DependencyResolverInterface::sort(ics.begin(), ics.end());
  }
  catch (CyclicDependencyException<DependencyResolverInterface *> & e)
  {
    std::ostringstream oss;

    oss << "Cyclic dependency detected in aux kernel ordering:" << std::endl;
    const std::multimap<DependencyResolverInterface *, DependencyResolverInterface *> & depends = e.getCyclicDependencies();
    for (std::multimap<DependencyResolverInterface *, DependencyResolverInterface *>::const_iterator it = depends.begin(); it != depends.end(); ++it)
      oss << (static_cast<ScalarInitialCondition *>(it->first))->name() << " -> " << (static_cast<ScalarInitialCondition *>(it->second))->name() << std::endl;
    mooseError(oss.str());
  }
}
