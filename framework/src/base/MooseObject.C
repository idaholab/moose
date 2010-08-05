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


MooseObject::MooseObject(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  _name(name),
  _moose_system(moose_system),
  _parameters(parameters),
  _tid(parameters.get<THREAD_ID>("_tid")),
  _use_displaced_mesh(moose_system.hasDisplacedMesh() && parameters.get<bool>("use_displaced_mesh"))
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

InputParameters &
MooseObject::parameters()
{
  return _parameters;
}

