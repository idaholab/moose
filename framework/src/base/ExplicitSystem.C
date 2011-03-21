#include "ExplicitSystem.h"
#include "SubProblem.h"

namespace Moose {

ExplicitSystem::ExplicitSystem(SubProblem & problem, const std::string & name) :
  SystemTempl<TransientExplicitSystem>(problem, name)
{
}

} // namespace
