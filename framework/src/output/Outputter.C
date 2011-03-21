#include "Outputter.h"
#include "Problem.h"

namespace Moose {

Outputter::Outputter(Problem & problem) :
  _problem(problem)
{
}

Outputter::~Outputter()
{
}

} // namespace
