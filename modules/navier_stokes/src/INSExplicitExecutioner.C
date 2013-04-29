#include "INSExplicitExecutioner.h"

template<>
InputParameters validParams<INSExplicitExecutioner>()
{
  InputParameters params = validParams<Transient>();

  return params;
}

INSExplicitExecutioner::INSExplicitExecutioner(const std::string & name, InputParameters parameters) :
  Transient(name, parameters),
  _dt_pps(getPostprocessorValue("dt"))
{
}

INSExplicitExecutioner::~INSExplicitExecutioner()
{
}

Real
INSExplicitExecutioner::computeDT()
{
  // Use the timestep returned by the postprocessor unless it is larger than dtmax
  return std::min(_dt_pps, _dtmax);
}
