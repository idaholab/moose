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

#include "TimePeriodOld.h"

TimePeriodOld::TimePeriodOld(const std::string & name, Real start) :
    _name(name),
    _start(start)
{}

void
TimePeriodOld::addActiveObjects(std::string kind, const std::vector<std::string> & object_list)
{
  _objects[kind] = object_list;
  _list_type[kind] = true;
}

void
TimePeriodOld::addInactiveObjects(std::string kind, const std::vector<std::string> & object_list)
{
  _objects[kind] = object_list;
  _list_type[kind] = false;
}

const std::vector<std::string> &
TimePeriodOld::getObjectList(const std::string & kind, bool & is_active)
{
  is_active = _list_type[kind];
  return _objects[kind];
}

const std::string &
TimePeriodOld::name() const
{
  return _name;
}

Real
TimePeriodOld::start() const
{
  return _start;
}
