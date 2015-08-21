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
  params.addParam<std::vector<std::string> >("depends_on", "The MeshModifiers that this modifier relies upon (i.e. must execute before this one)");

  params.registerBase("MeshModifier");

  return params;
}

MeshModifier::MeshModifier(const InputParameters & parameters) :
    MooseObject(parameters),
    Restartable(parameters, "MeshModifiers"),
    _mesh_ptr(NULL),
    _depends_on(getParam<std::vector<std::string> >("depends_on"))
{
}

MeshModifier::~MeshModifier()
{
}
