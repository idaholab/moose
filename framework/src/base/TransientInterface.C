#include "TransientInterface.h"
#include "SubProblem.h"

namespace Moose
{

TransientInterface::TransientInterface(InputParameters & parameters) :
    _ti_problem(*parameters.get<Moose::SubProblem *>("_problem")),
    _t(_ti_problem.time()),
    _t_step(_ti_problem.timeStep()),
    _dt(_ti_problem.dt())
{

}

TransientInterface::~TransientInterface()
{
}

} // namespace
