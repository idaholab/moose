/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

Real
PenetrationAux::computeValue()
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  if(pinfo)
    return pinfo->_distance;

  return -999999;
}
