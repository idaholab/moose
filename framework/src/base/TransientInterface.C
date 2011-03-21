#include "TransientInterface.h"
#include "Problem.h"

TransientInterface::TransientInterface(InputParameters & parameters) :
    _ti_problem(*parameters.get<Problem*>("_problem")),
    _t(_ti_problem.time()),
    _t_step(_ti_problem.timeStep()),
    _dt(_ti_problem.dt()),
    _time_weight(_ti_problem.timeWeights()),
    _is_transient(_ti_problem.transient())
{
}

TransientInterface::~TransientInterface()
{
}
