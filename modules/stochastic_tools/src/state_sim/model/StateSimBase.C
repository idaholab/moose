/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimBase.h"
#include <string>

const TimespanH StateSimBase::_DAY_TIME = 24;

StateSimBase::StateSimBase(const int & bit_id, const std::string &  name)
  : _id(bit_id),
    _name(name),
    _desc(""),
    _processed(false)
{
}

StateSimBase::~StateSimBase() //needed to make the class pure virtual, there are no other virtual functions
{}

const std::string
StateSimBase::desc() const
{
  return _desc;
}

