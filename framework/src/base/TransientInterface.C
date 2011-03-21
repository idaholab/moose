#include "TransientInterface.h"
#include "SubProblem.h"

TransientInterface::TransientInterface(InputParameters & parameters) :
    _ti_problem(*parameters.get<SubProblem *>("_problem")),
    _t(_ti_problem.time()),
    _t_step(_ti_problem.timeStep()),
    _dt(_ti_problem.dt()),
    _is_transient(_ti_problem.transient())
{
}

TransientInterface::~TransientInterface()
{
}
