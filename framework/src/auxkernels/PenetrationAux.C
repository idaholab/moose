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
#include "MooseSystem.h"

#include "mesh.h"

template<>
InputParameters validParams<PenetrationAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("paired_boundary", "The boundary to be penetrated");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

PenetrationAux::PenetrationAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   _penetration_locator(_moose_system, _mesh, parameters.get<std::vector<unsigned int> >("boundary"), parameters.get<unsigned int>("paired_boundary"))
{ 
}

void PenetrationAux::setup()
{
  _penetration_locator.detectPenetration();
}  

Real
PenetrationAux::computeValue()
{
  return _penetration_locator.penetrationDistance(_current_node->id());
}
