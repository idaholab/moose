#include "SolidWall.h"

registerMooseObject("THMApp", SolidWall);

InputParameters
SolidWall::validParams()
{
  InputParameters params = FlowConnection::validParams();
  return params;
}

SolidWall::SolidWall(const InputParameters & params) : FlowConnection(params)
{
  logError("Deprecated component. Use SolidWall1Phase or SolidWall2Phase instead.");
}
