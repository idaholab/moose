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

enum class Interfaces
{
  UserObject = 1 << 1,
  ElementUserObject = 1 << 2,
  SideUserObject = 1 << 3,
  InternalSideUserObject = 1 << 4,
  NodalUserObject = 1 << 5,
  GeneralUserObject = 1 << 6,
  ThreadedGeneralUserObject = 1 << 7,
  ShapeElementUserObject = 1 << 8,
  ShapeSideUserObject = 1 << 9,
  Postprocessor = 1 << 10,
  VectorPostprocessor = 1 << 11,
  InterfaceUserObject = 1 << 12,
};

template <>
struct enable_bitmask_operators<Interfaces>
{
  static const bool enable = true;
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
    hash_combine(h, __VA_ARGS__);                                                                  \
    return h;                                                                                      \
  }

class AttribTagBase : public Attribute
{
public:
  AttribTagBase(TheWarehouse & w, unsigned int tag, const std::string & attrib_name)
    : Attribute(w, attrib_name)
  {
    _vals.push_back(tag);
  }

  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_vals);

protected:
  std::vector<unsigned int> _vals;
};

class AttribMatrixTags : public AttribTagBase
{
public:
  AttribMatrixTags(TheWarehouse & w, unsigned int tag) : AttribTagBase(w, tag, "matrix_tags") {}
  virtual void initFrom(const MooseObject * obj) override;
  clonefunc(AttribMatrixTags);
};

class AttribVectorTags : public AttribTagBase
{
public:
  clonefunc(AttribVectorTags);

  AttribVectorTags(TheWarehouse & w, unsigned int tag) : AttribTagBase(w, tag, "vector_tags") {}
  virtual void initFrom(const MooseObject * obj) override;
};

class AttribExecOns : public Attribute
{
public:
  AttribExecOns(TheWarehouse & w, unsigned int exec_flag) : Attribute(w, "exec_ons")
  {
    _vals.push_back(exec_flag);
  }
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_vals);
  clonefunc(AttribExecOns);

private:
  std::vector<unsigned int> _vals;
};

class AttribSubdomains : public Attribute
{
public:
  AttribSubdomains(TheWarehouse & w, SubdomainID id) : Attribute(w, "subdomains")
  {
    _vals.push_back(id);
  }
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_vals);
  clonefunc(AttribSubdomains);

private:
  std::vector<SubdomainID> _vals;
};

class AttribBoundaries : public Attribute
{
public:
  AttribBoundaries(TheWarehouse & w, BoundaryID id, bool must_be_restricted = false)
    : Attribute(w, "boundaries"), _must_be_restricted(must_be_restricted)
  {
    _vals.push_back(id);
  }
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_vals, _must_be_restricted);
  clonefunc(AttribBoundaries);

private:
  std::vector<BoundaryID> _vals;
  bool _must_be_restricted;
};

class AttribThread : public Attribute
{
public:
  AttribThread(TheWarehouse & w, THREAD_ID t) : Attribute(w, "thread"), _val(t) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribThread);

private:
  THREAD_ID _val;
};

/// TODO: delete this later - it is a temporary hack for dealing with inter-system dependencies
class AttribPreIC : public Attribute
{
public:
  AttribPreIC(TheWarehouse & w, bool pre_ic) : Attribute(w, "pre_ic"), _val(pre_ic) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribPreIC);

private:
  bool _val;
};

/// TODO: delete this later - it is a temporary hack for dealing with inter-system dependencies
class AttribPreAux : public Attribute
{
public:
  AttribPreAux(TheWarehouse & w, bool pre_aux) : Attribute(w, "pre_aux"), _val(pre_aux) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribPreAux);

private:
  bool _val;
};

class AttribName : public Attribute
{
public:
  AttribName(TheWarehouse & w, const std::string & name) : Attribute(w, "name"), _val(name) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribName);

private:
  std::string _val;
};

class AttribSystem : public Attribute
{
public:
  AttribSystem(TheWarehouse & w, const std::string & system) : Attribute(w, "system"), _val(system)
  {
  }
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribSystem);

private:
  std::string _val;
};

class AttribVar : public Attribute
{
public:
  AttribVar(TheWarehouse & w, int var) : Attribute(w, "variable"), _val(var) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribVar);

private:
  int _val = -1;
};

class AttribInterfaces : public Attribute
{
public:
  AttribInterfaces(TheWarehouse & w, Interfaces mask)
    : Attribute(w, "interfaces"), _val(static_cast<uint64_t>(mask))
  {
  }
  AttribInterfaces(TheWarehouse & w, unsigned int mask) : Attribute(w, "interfaces"), _val(mask) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribInterfaces);

private:
  uint64_t _val = 0;
};

#undef clonefunc
#undef hashfunc

