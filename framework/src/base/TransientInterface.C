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
#include "Problem.h"

TransientInterface::TransientInterface(InputParameters & parameters) :
    _ti_problem(*parameters.get<Problem*>("_problem")),
    _t(_ti_problem.time()),
    _t_step(_ti_problem.timeStep()),
    _dt(_ti_problem.dt()),
    _dt_old(_ti_problem.dtOld()),
    _time_weight(_ti_problem.timeWeights()),
    _is_transient(_ti_problem.isTransient())
{
}

TransientInterface::~TransientInterface()
{
}
