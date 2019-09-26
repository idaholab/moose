#include "StabilizationSettings.h"

template <>
InputParameters
validParams<StabilizationSettings>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addPrivateParam<THMProblem *>("_thm_problem");
  params.registerBase("StabilizationSettings");
  return params;
}

StabilizationSettings::StabilizationSettings(const InputParameters & parameters)
  : GeneralUserObject(parameters), NamingInterface(), _m_factory(_app.getFactory())
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
