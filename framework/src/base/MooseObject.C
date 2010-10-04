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

#include "MooseObject.h"
#include "Moose.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<MooseObject>()
{
  InputParameters params;
  params.addPrivateParam<THREAD_ID>("_tid");
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  return params;
}


MooseObject::MooseObject(const std::string & name, MooseSystem & moose_system, InputParameters parameters) :
  _name(name),
  _moose_system(moose_system),
  _tid(parameters.get<THREAD_ID>("_tid")),
  _use_displaced_mesh(moose_system.hasDisplacedMesh() && parameters.get<bool>("use_displaced_mesh")),
  _parameters(parameters)
{}

MooseObject::~MooseObject()
{}

const std::string &
MooseObject::name()
{
  return _name;
}

THREAD_ID
MooseObject::tid()
{
  return _tid;
}
