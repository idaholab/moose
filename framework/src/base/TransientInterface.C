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

#include "TransientInterface.h"
#include "SubProblem.h"

TransientInterface::TransientInterface(InputParameters & parameters) :
    _ti_subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _t(_ti_subproblem.time()),
    _t_step(_ti_subproblem.timeStep()),
    _dt(_ti_subproblem.dt()),
    _dt_old(_ti_subproblem.dtOld()),
    _time_weight(_ti_subproblem.timeWeights()),
    _is_transient(_ti_subproblem.isTransient())
{
}

TransientInterface::~TransientInterface()
{
}
