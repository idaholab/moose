#include "TransientInterface.h"
#include "SubProblem.h"

namespace Moose
{

TransientInterface::TransientInterface(InputParameters & parameters) :
    _problem(*parameters.get<Moose::SubProblem *>("_problem")),
    _t(_problem.time()),
    _t_step(_problem.timeStep()),
    _dt(_problem.dt())
{

}

TransientInterface::~TransientInterface()
{
}

} // namespace
