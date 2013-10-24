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

#include "ZeroInterface.h"
#include "SubProblem.h"

ZeroInterface::ZeroInterface(InputParameters parameters) :
    _zi_subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _zi_tid(parameters.get<THREAD_ID>("_tid")),
    _real_zero(_zi_subproblem._real_zero[_zi_tid]),
    _zero(_zi_subproblem._zero[_zi_tid]),
    _grad_zero(_zi_subproblem._grad_zero[_zi_tid]),
    _second_zero(_zi_subproblem._second_zero[_zi_tid]),
    _second_phi_zero(_zi_subproblem._second_phi_zero[_zi_tid])
{
}
