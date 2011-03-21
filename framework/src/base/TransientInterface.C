#include "TransientInterface.h"
#include "SubProblem.h"

namespace Moose
{

TransientInterface::TransientInterface(InputParameters & parameters) :
    _subproblem(*parameters.get<Moose::SubProblem *>("_subproblem")),
    _t(_subproblem.time()),
    _t_step(_subproblem.timeStep()),
    _dt(_subproblem.dt())
{

}

TransientInterface::~TransientInterface()
{
}

}
