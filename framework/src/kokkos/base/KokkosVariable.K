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

  _moose_var.createHost(_components);
  _var.create(_components);
  _sys.create(_components);

  for (unsigned int comp = 0; comp < _components; ++comp)
  {
    _moose_var[comp] = &variable;
    _var[comp] = variable.number() + comp;
    _sys[comp] = variable.sys().number();
  }

  _var.copyToDevice();
  _sys.copyToDevice();

  const auto & problem = variable.sys().feProblem();

  if (problem.vectorTagExists(Moose::SOLUTION_DOT_TAG))
    _dot = tag == problem.getVectorTagID(Moose::SOLUTION_DOT_TAG);

  if (problem.vectorTagExists(Moose::OLD_SOLUTION_TAG))
    _old = tag == problem.getVectorTagID(Moose::OLD_SOLUTION_TAG);

  if (problem.vectorTagExists(Moose::OLDER_SOLUTION_TAG))
    _old = _old || tag == problem.getVectorTagID(Moose::OLDER_SOLUTION_TAG);
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

  _moose_var.createHost(_components);
  _var.create(_components);
  _sys.create(_components);

  for (unsigned int comp = 0; comp < _components; ++comp)
  {
    _nodal = _nodal && variables[comp]->isNodal();

    _moose_var[comp] = variables[comp];
    _var[comp] = variables[comp]->number();
    _sys[comp] = variables[comp]->sys().number();
  }

  _var.copyToDevice();
  _sys.copyToDevice();

  const auto & problem = variables[0]->sys().feProblem();

  if (problem.vectorTagExists(Moose::SOLUTION_DOT_TAG))
    _dot = tag == problem.getVectorTagID(Moose::SOLUTION_DOT_TAG);

  if (problem.vectorTagExists(Moose::OLD_SOLUTION_TAG))
    _old = tag == problem.getVectorTagID(Moose::OLD_SOLUTION_TAG);

  if (problem.vectorTagExists(Moose::OLDER_SOLUTION_TAG))
    _old = _old || tag == problem.getVectorTagID(Moose::OLDER_SOLUTION_TAG);
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

void
Variable::initBoundaryConditions(const std::vector<BoundaryID> & ids,
                                 const std::vector<Moose::Kokkos::Function> & values)
{
  mooseAssert(ids.size() == values.size(), "BC ids and values must have the same length");

  _boundary_ids.create(ids.size());
  _boundary_values.create<false>(values.size());

  for (std::size_t i = 0; i < ids.size(); ++i)
  {
    _boundary_ids[i] = ids[i];
    new (&_boundary_values[i]) Moose::Kokkos::Function(values[i]);
  }

  _boundary_ids.copyToDevice();
  _boundary_values.copyToDevice();
}

} // namespace Moose::Kokkos
