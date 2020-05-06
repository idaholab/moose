//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "Assembly.h"

class SubProblem;

class Reconstructor : public MooseObject
{
public:
  static InputParameters validParams();
  Reconstructor(const InputParameters & params);

  virtual ADReal gradient(const MooseVariableBase & var, const Elem * elem) = 0;
  virtual void coupledElements(std::vector<const Elem *> & elems) = 0;

protected:
  ADReal value(const MooseVariableBase & var, const Elem * elem)
  {
    var.getDofIndices(elem, _dof_indices);
    return 0;
  }
  SubProblem & _subproblem;
  THREAD_ID _tid;
  Assembly & _assembly;

private:
  std::vector<dof_id_type> _dof_indices;
};
