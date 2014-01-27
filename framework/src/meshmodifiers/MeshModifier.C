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

  params.registerBase("MeshModifier");

  return params;
}

MeshModifier::MeshModifier(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    Restartable(name, parameters, "MeshModifiers"),
    _mesh_ptr(NULL)
{
}

MeshModifier::~MeshModifier()
{
}
