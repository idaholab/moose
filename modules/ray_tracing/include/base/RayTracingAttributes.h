//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TheWarehouse.h"

class RayTracingStudy;

/**
 * Attribute for the RayTracingStudy a RayTracingObject is associated with.
 */
class AttribRayTracingStudy : public Attribute
{
public:
  AttribRayTracingStudy(TheWarehouse & w, const RayTracingStudy * study)
    : Attribute(w, "ray_tracing_study")
  {
    _val = study;
  }
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  virtual std::unique_ptr<Attribute> clone() const override
  {
    return std::unique_ptr<Attribute>(new AttribRayTracingStudy(*this));
  }
  virtual size_t hash() const override
  {
    size_t h = 0;
    Moose::hash_combine(h, _val);
    return h;
  }

private:
  const RayTracingStudy * _val;
};
