#include "Problem.h"

namespace Moose
{

Problem::Problem() :
    _out(*this)
{
}

Problem::~Problem()
{
}

void
Problem::output()
{
  _out.output();
}


} // namespace
