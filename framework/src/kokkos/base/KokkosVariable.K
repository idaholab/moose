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
Variable::init(const MooseVariableBase & variable, const TagID tag)
{
  auto field_variable = dynamic_cast<const MooseVariableFieldBase *>(&variable);
  auto scalar_variable = dynamic_cast<const MooseVariableScalar *>(&variable);

  _initialized = true;
  _coupled = true;
  _nodal = field_variable ? field_variable->isNodal() : false;
  _scalar = scalar_variable ? true : false;
  _components = variable.count();
  _tag = tag;
  _vector = field_variable ? field_variable->isVector() : false;

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
Variable::init(const std::vector<const MooseVariableBase *> & variables, const TagID tag)
{
  if (variables.empty())
    mooseError("Cannot initialize a Kokkos variable with zero components.");

  auto field_variable = dynamic_cast<const MooseVariableFieldBase *>(variables[0]);
  auto scalar_variable = dynamic_cast<const MooseVariableScalar *>(variables[0]);

  _initialized = true;
  _coupled = true;
  _nodal = true;
  _scalar = scalar_variable ? true : false;
  _components = variables.size();
  _tag = tag;
  _vector = field_variable ? field_variable->isVector() : false;

  _moose_var.createHost(_components);
  _var.create(_components);
  _sys.create(_components);

  for (unsigned int comp = 0; comp < _components; ++comp)
  {
    auto field_variable = dynamic_cast<const MooseVariableFieldBase *>(variables[comp]);

    if (field_variable && _scalar)
      mooseError("Cannot initialize a Kokkos variable with mixed scalar and field variables.");

    if (field_variable && field_variable->isVector() != _vector)
      mooseError("Cannot initialize a Kokkos variable with mixed scalar and vector components.");

    _nodal = _nodal && (field_variable ? field_variable->isNodal() : false);

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
  _nodal = false;
  _components = values.size();

  _default_value = values;
}

void
Variable::init(const std::vector<Real> & values, ScalarCoupleableKey)
{
  _initialized = true;
  _coupled = false;
  _nodal = false;
  _scalar = true;
  _components = 1;

  _default_value = values;
}

void
Variable::init(const std::vector<Real3> & values, CoupleableKey)
{
  _initialized = true;
  _coupled = false;
  _nodal = false;
  _components = values.size();
  _vector = true;

  _default_vector_value = values;
}

void
Variable::init(const MooseVariableBase & variable, const TagName & tag_name)
{
  init(variable, variable.sys().feProblem().getVectorTagID(tag_name));
}

void
Variable::init(const std::vector<const MooseVariableBase *> & variables, const TagName & tag_name)
{
  init(variables, variables[0]->sys().feProblem().getVectorTagID(tag_name));
}

void
Variable::init(const std::vector<MooseVariableBase *> & variables, const TagName & tag_name)
{
  init(variables, variables[0]->sys().feProblem().getVectorTagID(tag_name));
}

void
Variable::init(const std::vector<const MooseVariableFieldBase *> & variables,
               const TagName & tag_name)
{
  init(variables, variables[0]->sys().feProblem().getVectorTagID(tag_name));
}

void
Variable::init(const std::vector<MooseVariableFieldBase *> & variables, const TagName & tag_name)
{
  init(variables, variables[0]->sys().feProblem().getVectorTagID(tag_name));
}

void
Variable::init(const std::vector<const MooseVariableScalar *> & variables, const TagName & tag_name)
{
  init(variables, variables[0]->sys().feProblem().getVectorTagID(tag_name));
}

void
Variable::init(const std::vector<MooseVariableScalar *> & variables, const TagName & tag_name)
{
  init(variables, variables[0]->sys().feProblem().getVectorTagID(tag_name));
}

} // namespace Moose::Kokkos
