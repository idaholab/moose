#include "FreeBoundary.h"

registerMooseObject("ThermalHydraulicsApp", FreeBoundary);

InputParameters
FreeBoundary::validParams()
{
  InputParameters params = FlowConnection::validParams();
  return params;
}

FreeBoundary::FreeBoundary(const InputParameters & parameters) : FlowConnection(parameters)
{
  logError("Deprecated component. Use FreeBoundary1Phase or FreeBoundary2Phase instead.");
}
