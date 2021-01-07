//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Attributes.h"

enum class INSFVBCs
{
  INSFVFlowBC = 1 << 1,
  INSFVFullyDevelopedFlowBC = 1 << 2,
  INSFVNoSlipWallBC = 1 << 3,
  INSFVSlipWallBC = 1 << 4,
  INSFVSymmetryBC = 1 << 5
};

#define clonefunc(T)                                                                               \
  virtual std::unique_ptr<Attribute> clone() const override                                        \
  {                                                                                                \
    return std::unique_ptr<Attribute>(new T(*this));                                               \
  }

#define hashfunc(...)                                                                              \
  virtual size_t hash() const override                                                             \
  {                                                                                                \
    size_t h = 0;                                                                                  \
    Moose::hash_combine(h, __VA_ARGS__);                                                           \
    return h;                                                                                      \
  }

class AttribINSFVBCs : public Attribute
{
public:
  typedef INSFVBCs Key;
  void setFrom(Key k) { _val = static_cast<uint64_t>(k); }

  AttribINSFVBCs(TheWarehouse & w) : Attribute(w, "insfvbcs") {}
  AttribINSFVBCs(TheWarehouse & w, INSFVBCs mask)
    : Attribute(w, "insfvbcs"), _val(static_cast<uint64_t>(mask))
  {
  }
  AttribINSFVBCs(TheWarehouse & w, unsigned int mask) : Attribute(w, "insfvbcs"), _val(mask) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribINSFVBCs);

private:
  uint64_t _val = 0;
};

#undef clonefunc
#undef hashfunc
