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

#include "MeshModifier.h"

template<>
InputParameters validParams<MeshModifier>()
{
  InputParameters params = validParams<MooseObject>();

  params.addPrivateParam<std::string>("built_by_action", "add_mesh_modifier");
  return params;
}

MeshModifier::MeshModifier(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters)
{
}

