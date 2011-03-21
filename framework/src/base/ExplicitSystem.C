#include "ExplicitSystem.h"
#include "SubProblem.h"

namespace Moose {

ExplicitSystem::ExplicitSystem(ProblemInterface & problem, const std::string & name) :
    SystemTempl<TransientExplicitSystem>(problem, name)
{
}

} // namespace
