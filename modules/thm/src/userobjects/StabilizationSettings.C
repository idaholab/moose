#include "StabilizationSettings.h"
#include "Simulation.h"

template <>
InputParameters
validParams<StabilizationSettings>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addPrivateParam<Simulation *>("_sim");
  params.registerBase("StabilizationSettings");
  return params;
}

StabilizationSettings::StabilizationSettings(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _m_sim(*getCheckedPointerParam<Simulation *>("_sim")),
    _m_app(_m_sim.getApp()),
    _m_factory(_m_app.getFactory())
{
}

void
StabilizationSettings::execute()
{
}

void
StabilizationSettings::initialize()
{
}

void
StabilizationSettings::finalize()
{
}
