//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ZeroInterface.h"
#include "FEProblem.h"

ZeroInterface::ZeroInterface(const InputParameters & parameters)
  : _zi_feproblem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _zi_tid(parameters.get<THREAD_ID>("_tid")),
    _real_zero(_zi_feproblem._real_zero[_zi_tid]),
    _zero(_zi_feproblem._zero[_zi_tid]),
    _grad_zero(_zi_feproblem._grad_zero[_zi_tid]),
    _second_zero(_zi_feproblem._second_zero[_zi_tid]),
    _second_phi_zero(_zi_feproblem._second_phi_zero[_zi_tid])
{
}
