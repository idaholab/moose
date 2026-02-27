//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVariable.h"

#include "SystemBase.h"
#include "FEProblemBase.h"

namespace Moose::Kokkos
{

void
Variable::init(const MooseVariableFieldBase & variable, const TagName & tag_name)
{
  init(variable, variable.sys().feProblem().getVectorTagID(tag_name));
}

void
Variable::init(const MooseVariableFieldBase & variable, const TagID tag)
{
  _initialized = true;
  _coupled = true;
  _nodal = variable.isNodal();
  _components = variable.count();
  _tag = tag;

  _var.create(_components);
  _sys.create(_components);

  for (unsigned int comp = 0; comp < _components; ++comp)
  {
    _var[comp] = variable.number() + comp;
    _sys[comp] = variable.sys().number();
  }

  _var.copyToDevice();
  _sys.copyToDevice();
}

void
Variable::init(const std::vector<MooseVariableFieldBase *> & variables, const TagName & tag_name)
{
  init(std::vector<const MooseVariableFieldBase *>(variables.begin(), variables.end()),
       variables[0]->sys().feProblem().getVectorTagID(tag_name));
}

void
Variable::init(const std::vector<const MooseVariableFieldBase *> & variables,
               const TagName & tag_name)
{
  init(variables, variables[0]->sys().feProblem().getVectorTagID(tag_name));
}

void
Variable::init(const std::vector<MooseVariableFieldBase *> & variables, const TagID tag)
{
  init(std::vector<const MooseVariableFieldBase *>(variables.begin(), variables.end()), tag);
}

void
Variable::init(const std::vector<const MooseVariableFieldBase *> & variables, const TagID tag)
{
  _initialized = true;
  _coupled = true;
  _nodal = true;
  _components = variables.size();
  _tag = tag;

  _var.create(_components);
  _sys.create(_components);

  for (unsigned int comp = 0; comp < _components; ++comp)
  {
    _nodal = _nodal && variables[comp]->isNodal();

    _var[comp] = variables[comp]->number();
    _sys[comp] = variables[comp]->sys().number();
  }

  _var.copyToDevice();
  _sys.copyToDevice();
}

void
Variable::init(const std::vector<Real> & values, CoupleableKey)
{
  _initialized = true;
  _coupled = false;
  _nodal = true;
  _components = values.size();

  _default_value = values;
}

} // namespace Moose::Kokkos
