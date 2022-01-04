#include "JunctionOneToOne.h"

registerMooseObject("THMApp", JunctionOneToOne);

InputParameters
JunctionOneToOne::validParams()
{
  InputParameters params = FlowJunction::validParams();

  params.addClassDescription("Junction connecting one flow channel to one other flow channel");

  return params;
}

JunctionOneToOne::JunctionOneToOne(const InputParameters & params) : FlowJunction(params)
{
  logError("Depreacted component. Use JunctionOneToOne1Phase or JunctionOneToOne2Phase instead.");
}
