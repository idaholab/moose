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
  for (std::map<std::string, ScalarInitialCondition *>::iterator it = _scalar_ics.begin(); it != _scalar_ics.end(); ++it)
    delete it->second;
}

void
InitialConditionWarehouse::initialSetup()
{
  for (std::map<std::string, std::map<SubdomainID, InitialCondition *> >::iterator it1 = _ics.begin(); it1 != _ics.end(); ++it1)
    for (std::map<SubdomainID, InitialCondition *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
      it2->second->initialSetup();
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
    mooseError(std::string("Initial Conditions '") + name + "' and '" + ic->name()
               + "' are both defined on the same block.");

  _ics[var_name][blockid] = ic;
}

InitialCondition *
InitialConditionWarehouse::getInitialCondition(const std::string & var_name, SubdomainID blockid)
{
  std::map<std::string, std::map<SubdomainID, InitialCondition *> >::iterator it = _ics.find(var_name);
  if (it != _ics.end())
  {
    std::map<SubdomainID, InitialCondition *>::iterator jt = it->second.find(blockid);
    if (jt != it->second.end())
      return jt->second;                        // we return the IC that was defined on the specified block (blockid)

    jt = it->second.find(Moose::ANY_BLOCK_ID);
    if (jt != it->second.end())
      return jt->second;                        // return the IC that lives everywhere
    else
      return NULL;                              // No IC there at all
  }
  else
    return NULL;
}


void
InitialConditionWarehouse::addScalarInitialCondition(const std::string & var_name, ScalarInitialCondition * ic)
{
  std::map<std::string, ScalarInitialCondition *>::iterator it = _scalar_ics.find(var_name);
  if (it == _scalar_ics.end())
    _scalar_ics[var_name] = ic;
  else
    mooseError("Initial condition for variable '" << var_name << "' has been already set.");
}

ScalarInitialCondition *
InitialConditionWarehouse::getScalarInitialCondition(const std::string & var_name)
{
  std::map<std::string, ScalarInitialCondition *>::iterator it = _scalar_ics.find(var_name);
  if (it != _scalar_ics.end())
    return it->second;
  else
    return NULL;
}
