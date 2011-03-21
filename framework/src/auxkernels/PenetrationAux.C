#include "PenetrationAux.h"

template<>
InputParameters validParams<PenetrationAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("paired_boundary", "The boundary to be penetrated");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

PenetrationAux::PenetrationAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _penetration_locator(getPenetrationLocator(parameters.get<unsigned int>("paired_boundary"), getParam<std::vector<unsigned int> >("boundary")[0]))
{ 
}

void PenetrationAux::setup()
{}  

Real
PenetrationAux::computeValue()
{
  Moose::PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  /*
  if(_penetration_locator._has_penetrated[_current_node->id()])
    return 1;
  else
    return 0;
  */

  if(pinfo)
    return pinfo->_distance;
  

  return -999999;
}
