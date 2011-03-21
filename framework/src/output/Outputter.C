#include "Outputter.h"
#include "Problem.h"

namespace Moose {

Outputter::Outputter(EquationSystems & es) :
    _es(es)
{
}

Outputter::~Outputter()
{
}

} // namespace
