#include "ExplicitSystem.h"
#include "Problem.h"

namespace Moose {

ExplicitSystem::ExplicitSystem(Problem & problem, const std::string & name) :
  SubProblemTempl<TransientExplicitSystem>(problem, name)
{
//  _eq.parameters.set<ExplicitSystem>("_sys") = this;

}

} // namespace
