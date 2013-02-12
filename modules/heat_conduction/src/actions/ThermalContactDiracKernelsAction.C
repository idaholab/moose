#include "ThermalContactDiracKernelsAction.h"

#include "Factory.h"

template<>
InputParameters validParams<ThermalContactDiracKernelsAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  return params;
}

ThermalContactDiracKernelsAction::ThermalContactDiracKernelsAction(const std::string & name, InputParameters params) :
  Action(name, params)
{
}

void
ThermalContactDiracKernelsAction::act()
{
  InputParameters params = _factory.getValidParams("GapHeatPointSourceMaster");
  params.set<NonlinearVariableName>("variable") = getParam<NonlinearVariableName>("variable");
  params.set<BoundaryName>("boundary") = getParam<BoundaryName>("master");
  params.set<BoundaryName>("slave") = getParam<BoundaryName>("slave");
  if (isParamValid("tangential_tolerance"))
  {
    params.set<Real>("tangential_tolerance") = getParam<Real>("tangential_tolerance");
  }

  _problem->addDiracKernel("GapHeatPointSourceMaster",
                           "GapHeatPointSourceMaster_"+_name,
                           params);
}
