#include "ExplicitSystem.h"
#include "SubProblem.h"

namespace Moose {

ExplicitSystem::ExplicitSystem(Problem & problem, const std::string & name) :
    SystemTempl<TransientExplicitSystem>(problem, name)
{
}

} // namespace
