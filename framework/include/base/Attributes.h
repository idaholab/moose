//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseHashing.h"
#include "TheWarehouse.h"

#include <ostream>
#include <tuple>
#include <type_traits>

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
  BlockRestrictable = 1 << 13,
  BoundaryRestrictable = 1 << 14,
  Reporter = 1 << 15
};

template <>
struct enable_bitmask_operators<Interfaces>
{
  static const bool enable = true;
};

std::ostream & operator<<(std::ostream & os, Interfaces & iface);

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

/// AttribTagBase tracks all (vector or matrix) tags associated with an object.
/// When running queries, an object matches true if it has at least one tag in
/// common with the tags in the query attribute.
class AttribTagBase : public Attribute
{
public:
  AttribTagBase(TheWarehouse & w, const std::string & attrib_name) : Attribute(w, attrib_name) {}
  AttribTagBase(TheWarehouse & w, TagID tag, const std::string & attrib_name)
    : Attribute(w, attrib_name)
  {
    _vals.push_back(tag);
  }
  AttribTagBase(TheWarehouse & w, const std::set<TagID> & tags, const std::string & attrib_name)
    : Attribute(w, attrib_name)
  {
    for (auto tag : tags)
      _vals.push_back(tag);
  }

  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_vals);

protected:
  std::vector<TagID> _vals;
};

class AttribMatrixTags : public AttribTagBase
{
public:
  typedef unsigned int Key;
  void setFrom(Key k)
  {
    _vals.clear();
    _vals.push_back(k);
  }

  AttribMatrixTags(TheWarehouse & w) : AttribTagBase(w, "matrix_tags") {}
  AttribMatrixTags(TheWarehouse & w, TagID tag) : AttribTagBase(w, tag, "matrix_tags") {}
  AttribMatrixTags(TheWarehouse & w, const std::set<TagID> & tags)
    : AttribTagBase(w, tags, "matrix_tags")
  {
  }
  virtual void initFrom(const MooseObject * obj) override;
  clonefunc(AttribMatrixTags);
};

class AttribVectorTags : public AttribTagBase
{
public:
  clonefunc(AttribVectorTags);

  typedef unsigned int Key;
  void setFrom(Key k)
  {
    _vals.clear();
    _vals.push_back(k);
  }

  AttribVectorTags(TheWarehouse & w) : AttribTagBase(w, "vector_tags") {}
  AttribVectorTags(TheWarehouse & w, TagID tag) : AttribTagBase(w, tag, "vector_tags") {}
  AttribVectorTags(TheWarehouse & w, const std::set<TagID> & tags)
    : AttribTagBase(w, tags, "vector_tags")
  {
  }
  virtual void initFrom(const MooseObject * obj) override;
};

class AttribExecOns : public Attribute
{
public:
  typedef unsigned int Key;
  void setFrom(Key k)
  {
    _vals.clear();
    _vals.push_back(k);
  }

  AttribExecOns(TheWarehouse & w) : Attribute(w, "exec_ons") {}
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
  typedef SubdomainID Key;
  void setFrom(Key k)
  {
    _vals.clear();
    _vals.push_back(k);
  }

  AttribSubdomains(TheWarehouse & w) : Attribute(w, "subdomains") {}
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

/// AttribBoundaries tracks all boundary IDs associated with an object.
/// When running queries, an object matches true if it has at least one
/// boundary id in common with the boundary IDs in the query attribute.
class AttribBoundaries : public Attribute
{
public:
  typedef std::tuple<BoundaryID, bool> Key;
  void setFrom(Key k)
  {
    _vals.clear();
    _vals.push_back(std::get<0>(k));
    _must_be_restricted = std::get<1>(k);
  }

  AttribBoundaries(TheWarehouse & w) : Attribute(w, "boundaries") {}
  AttribBoundaries(TheWarehouse & w, BoundaryID id, bool must_be_restricted = false)
    : Attribute(w, "boundaries"), _must_be_restricted(must_be_restricted)
  {
    _vals.push_back(id);
  }
  AttribBoundaries(TheWarehouse & w,
                   const std::set<BoundaryID> & ids,
                   bool must_be_restricted = false)
    : Attribute(w, "boundaries"), _must_be_restricted(must_be_restricted)
  {
    for (auto id : ids)
      _vals.push_back(id);
  }
  AttribBoundaries(TheWarehouse & w,
                   const std::vector<BoundaryID> & ids,
                   bool must_be_restricted = false)
    : Attribute(w, "boundaries"), _must_be_restricted(must_be_restricted)
  {
    _vals.reserve(ids.size());
    for (auto id : ids)
      _vals.push_back(id);
  }
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_vals, _must_be_restricted);
  clonefunc(AttribBoundaries);

private:
  std::vector<BoundaryID> _vals;
  bool _must_be_restricted = false;
};

class AttribThread : public Attribute
{
public:
  typedef THREAD_ID Key;
  void setFrom(Key k) { _val = k; }

  AttribThread(TheWarehouse & w) : Attribute(w, "thread") {}
  AttribThread(TheWarehouse & w, THREAD_ID t) : Attribute(w, "thread"), _val(t) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribThread);

private:
  THREAD_ID _val = 0;
};

/**
 * Tracks the libmesh system number that a \p MooseObject is associated with
 */
class AttribSysNum : public Attribute
{
public:
  typedef unsigned int Key;
  void setFrom(Key k) { _val = k; }

  AttribSysNum(TheWarehouse & w) : Attribute(w, "sys_num") {}
  AttribSysNum(TheWarehouse & w, unsigned int t) : Attribute(w, "sys_num"), _val(t) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribSysNum);

private:
  unsigned int _val = libMesh::invalid_uint;
};

/// TODO: delete this later - it is a temporary hack for dealing with inter-system dependencies
class AttribPreIC : public Attribute
{
public:
  typedef bool Key;
  void setFrom(Key k) { _val = k; }

  AttribPreIC(TheWarehouse & w) : Attribute(w, "pre_ic") {}
  AttribPreIC(TheWarehouse & w, bool pre_ic) : Attribute(w, "pre_ic"), _val(pre_ic) {}
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribPreIC);

private:
  bool _val = false;
};

/// TODO: delete this later - it is a temporary hack for dealing with inter-system dependencies
class AttribPreAux : public Attribute
{
public:
  typedef unsigned int Key;
  void setFrom(Key k)
  {
    _vals.clear();
    _vals.insert(k);
  }

  AttribPreAux(TheWarehouse & w) : Attribute(w, "pre_aux") {}
  AttribPreAux(TheWarehouse & w, unsigned int val) : Attribute(w, "pre_aux") { _vals.insert(val); }
  AttribPreAux(TheWarehouse & w, const std::set<unsigned int> & vals)
    : Attribute(w, "pre_aux"), _vals(vals)
  {
  }
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_vals);
  clonefunc(AttribPreAux);

private:
  std::set<unsigned int> _vals;
};

/// TODO: delete this later - it is a temporary hack for dealing with inter-system dependencies
///
/// this attribute was added to ensure that UOs are uniquely assigned a single group to
/// prevent multiple executions when it is queried in FEProblemBase::computeUserObjectsInternal()
/// for a given exec flag time.
///
class AttribPostAux : public Attribute
{
public:
  typedef unsigned int Key;
  void setFrom(Key k)
  {
    _vals.clear();
    _vals.insert(k);
  }

  AttribPostAux(TheWarehouse & w) : Attribute(w, "post_aux") {}
  AttribPostAux(TheWarehouse & w, unsigned int val) : Attribute(w, "post_aux")
  {
    _vals.insert(val);
  }
  AttribPostAux(TheWarehouse & w, const std::set<unsigned int> & vals)
    : Attribute(w, "post_aux"), _vals(vals)
  {
  }
  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_vals);
  clonefunc(AttribPostAux);

private:
  std::set<unsigned int> _vals;
};

class AttribName : public Attribute
{
public:
  typedef std::string Key;
  void setFrom(const Key & k) { _val = k; }

  AttribName(TheWarehouse & w) : Attribute(w, "name") {}
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
  typedef std::string Key;
  void setFrom(const Key & k) { _val = k; }

  AttribSystem(TheWarehouse & w) : Attribute(w, "system") {}
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

/**
 * Residual objects have this attribute
 */
class AttribResidualObject : public Attribute
{
public:
  typedef bool Key;
  void setFrom(const Key & k) { _val = k; }

  AttribResidualObject(TheWarehouse & w)
    : Attribute(w, "residual_object"), _val(false), _initd(false)
  {
  }

  AttribResidualObject(TheWarehouse & w, bool is_residual_object)
    : Attribute(w, "residual_object"), _val(is_residual_object), _initd(true)
  {
  }

  virtual void initFrom(const MooseObject * obj) override;
  virtual bool isMatch(const Attribute & other) const override;
  virtual bool isEqual(const Attribute & other) const override;
  hashfunc(_val);
  clonefunc(AttribResidualObject);

private:
  bool _val;
  bool _initd;
};

class AttribVar : public Attribute
{
public:
  typedef int Key;
  void setFrom(const Key & k) { _val = k; }

  AttribVar(TheWarehouse & w) : Attribute(w, "variable") {}
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
  typedef Interfaces Key;
  void setFrom(Key k) { _val = static_cast<uint64_t>(k); }

  AttribInterfaces(TheWarehouse & w) : Attribute(w, "interfaces") {}
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
