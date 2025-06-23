//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUVariable.h"

#include "SystemBase.h"
#include "FEProblemBase.h"

void
GPUVariable::init(const MooseVariableBase & variable, const TagName & tag_name)
{
  init(variable, variable.sys().feProblem().getVectorTagID(tag_name));
}

void
GPUVariable::init(const MooseVariableBase & variable, const TagID tag)
{
  _coupled = true;
  _components = variable.count();
  _tag = tag;

  _var.create(_components);
  _sys.create(_components);

  for (unsigned int i = 0; i < variable.count(); ++i)
  {
    _var[i] = variable.number() + i;
    _sys[i] = variable.sys().number();
  }

  _var.copy();
  _sys.copy();
}
