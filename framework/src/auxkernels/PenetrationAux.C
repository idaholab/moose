#include "PenetrationAux.h"
#include "MooseSystem.h"

#include "mesh.h"

template<>
InputParameters validParams<PenetrationAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("paired_boundary", "The boundary to be penetrated");
  return params;
}

PenetrationAux::PenetrationAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   _penetration_locator(*moose_system.getMesh(), parameters.get<std::vector<unsigned int> >("boundary"), parameters.get<unsigned int>("paired_boundary"))
{ 
  // For now we are working with meshes that do not move which means we can detect penetration once!
  _penetration_locator.detectPenetration();
}

Real
PenetrationAux::computeValue()
{
  return _penetration_locator.penetrationDistance(_current_node->id());
}
