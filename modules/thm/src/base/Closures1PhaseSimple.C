#include "Closures1PhaseSimple.h"
#include "Pipe.h"

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
Closures1PhaseSimple::check(const Pipe & flow_channel) const
{
  if (!flow_channel.isParamValid("f"))
    logError("When using simple closures, the parameter 'f' must be provided.");
}

void
Closures1PhaseSimple::addMooseObjects(const Pipe & flow_channel)
{
  // wall friction material
  addWallFrictionFunctionMaterial(flow_channel);
}
