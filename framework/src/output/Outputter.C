#include "Outputter.h"
#include "SubProblem.h"

namespace Moose {

Outputter::Outputter(SubProblem & problem) :
  _problem(problem)
{
}

Outputter::~Outputter()
{
}

} // namespace
