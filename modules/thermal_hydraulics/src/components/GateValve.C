#include "GateValve.h"
#include "GeometricalFlowComponent.h"
#include "FlowModelSinglePhase.h"
#include "THMMesh.h"

registerMooseObject("ThermalHydraulicsApp", GateValve);

InputParameters
GateValve::validParams()
{
  InputParameters params = FlowJunction::validParams();

  params.addRequiredParam<Real>("open_area_fraction", "Fraction of flow area that is open [-]");

  params.addClassDescription("Gate valve component");

  return params;
}

GateValve::GateValve(const InputParameters & params) : FlowJunction(params)
{
  logError("Deprecated component. Use GateValve or GateValve2Phase instead.");
}
