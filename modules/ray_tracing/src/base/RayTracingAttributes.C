//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingAttributes.h"

// Local includes
#include "RayTracingObject.h"

void
AttribRayTracingStudy::initFrom(const MooseObject * obj)
{
  const RayTracingObject * rto = dynamic_cast<const RayTracingObject *>(obj);
  if (rto)
    _val = &rto->study();
}

bool
AttribRayTracingStudy::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribRayTracingStudy *>(&other);
  return a && (a->_val == _val);
}

bool
AttribRayTracingStudy::isEqual(const Attribute & other) const
{
  return isMatch(other);
}
