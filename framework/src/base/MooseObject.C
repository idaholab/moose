#include "MooseObject.h"
#include "Moose.h"
#include "MooseSystem.h"

MooseObject::MooseObject(std::string name, MooseSystem & moose_system, InputParameters parameters) :
  _name(name),
  _moose_system(moose_system),
  _parameters(parameters),
  _tid(Moose::current_thread_id)
{
}

MooseObject::~MooseObject()
{
}

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
