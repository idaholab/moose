#include "ClosuresBase.h"

template <>
InputParameters
validParams<ClosuresBase>()
{
  InputParameters params = validParams<MooseObject>();

  params.addPrivateParam<Simulation *>("_sim");

  params.registerBase("THM:closures");

  return params;
}

ClosuresBase::ClosuresBase(const InputParameters & params)
  : MooseObject(params),
    LoggingInterface(dynamic_cast<THMApp &>(getMooseApp()), name()),

    _sim(*params.getCheckedPointerParam<Simulation *>("_sim")),
    _factory(_app.getFactory())
{
}
