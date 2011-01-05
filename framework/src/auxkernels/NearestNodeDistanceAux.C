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

#include "NearestNodeDistanceAux.h"
#include "MooseSystem.h"

#include "mesh.h"

template<>
InputParameters validParams<NearestNodeDistanceAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("paired_boundary", "The boundary to find the distance to.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

NearestNodeDistanceAux::NearestNodeDistanceAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _nearest_node(getNearestNodeLocator(parameters.get<unsigned int>("paired_boundary"), getParam<std::vector<unsigned int> >("boundary")[0]))
{
  if(getParam<std::vector<unsigned int> >("boundary").size() > 1)
    mooseError("NearestNodeDistanceAux can only be used with one boundary at a time!");
}

void NearestNodeDistanceAux::setup()
{
}

Real
NearestNodeDistanceAux::computeValue()
{
  return _nearest_node.distance(_current_node->id());
}
