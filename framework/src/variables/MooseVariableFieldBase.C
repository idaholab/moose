//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFieldBase.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "libmesh/system.h"

InputParameters
MooseVariableFieldBase::validParams()
{
  return MooseVariableBase::validParams();
}

MooseVariableFieldBase::MooseVariableFieldBase(const InputParameters & parameters)
  : MooseVariableBase(parameters)
{
}

std::string
MooseVariableFieldBase::componentName(const unsigned int comp) const
{
  if (comp >= _count)
    mooseError("Component index must be less than the number of components of variable ",
               _var_name);
  if (isArray())
    return this->_subproblem.arrayVariableComponent(_var_name, comp);
  else
    return _var_name;
}

const std::set<SubdomainID> &
MooseVariableFieldBase::activeSubdomains() const
{
  return this->_sys.system().variable(_var_num).active_subdomains();
}

bool
MooseVariableFieldBase::activeOnSubdomain(SubdomainID subdomain) const
{
  return this->_sys.system().variable(_var_num).active_on_subdomain(subdomain);
}

bool
MooseVariableFieldBase::activeOnSubdomains(const std::set<SubdomainID> & subdomains) const
{
  const auto & active_subs = activeSubdomains();
  return std::includes(
      active_subs.begin(), active_subs.end(), subdomains.begin(), subdomains.end());
}
