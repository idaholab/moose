#include "Closures1PhaseSimple.h"

registerMooseObject("THMApp", Closures1PhaseSimple);

template <>
InputParameters
validParams<Closures1PhaseSimple>()
{
  InputParameters params = validParams<Closures1PhaseBase>();

  params.addClassDescription("Simple 1-phase closures");

  return params;
}

Closures1PhaseSimple::Closures1PhaseSimple(const InputParameters & params)
  : Closures1PhaseBase(params)
{
}

void
Closures1PhaseSimple::check(const Pipe & /*flow_channel*/) const
{
}

void
Closures1PhaseSimple::addMooseObjects(const Pipe & /*flow_channel*/)
{
}
