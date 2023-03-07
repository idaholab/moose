//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TheWarehouse.h"

#include "Attributes.h"
#include "MooseObject.h"
#include "SubProblem.h"
#include "GeneralUserObject.h"
#include "DependencyResolverInterface.h"
#include "BlockRestrictable.h"

#include <memory>

class Storage
{
public:
  virtual ~Storage() = default;
  virtual void add(std::size_t obj_id, std::vector<std::unique_ptr<Attribute>> attribs) = 0;
  virtual std::vector<std::size_t> query(const std::vector<std::unique_ptr<Attribute>> & conds) = 0;
  virtual void set(std::size_t obj_id, std::vector<std::unique_ptr<Attribute>> attribs) = 0;
};

bool
operator==(const std::unique_ptr<Attribute> & lhs, const std::unique_ptr<Attribute> & rhs)
{
  return (*lhs) == (*rhs);
}

Attribute::Attribute(TheWarehouse & w, const std::string name) : _id(w.attribID(name)) {}

void
AttribSorted::initFrom(const MooseObject *)
{
}

bool
AttribSorted::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribSorted *>(&other);
  return _initd && a && a->_initd && (a->_val == _val);
}

bool
AttribSorted::isEqual(const Attribute & other) const
{
  return isMatch(other);
}

class VecStore : public Storage
{
public:
  virtual void add(std::size_t obj_id, std::vector<std::unique_ptr<Attribute>> attribs) override
  {
    std::lock_guard<std::mutex> l(_mutex);
    if (obj_id != _data.size())
      throw std::runtime_error("object with id " + std::to_string(obj_id) + " already added");
    _data.push_back(std::move(attribs));
  }

  virtual std::vector<std::size_t>
  query(const std::vector<std::unique_ptr<Attribute>> & conds) override
  {
    std::vector<std::size_t> ids;
    std::lock_guard<std::mutex> l(_mutex);
    for (std::size_t i = 0; i < _data.size(); i++)
    {
      auto & data = _data[i];
      bool ismatch = true;
      for (auto & cond : conds)
      {
        if (!data[cond->id()]->isMatch(*cond))
        {
          ismatch = false;
          break;
        }
      }
      if (ismatch)
        ids.push_back(i);
    }
    return ids;
  }

  virtual void set(std::size_t obj_id, std::vector<std::unique_ptr<Attribute>> attribs) override
  {
    if (obj_id > _data.size())
      throw std::runtime_error("unknown object id " + std::to_string(obj_id));

    std::lock_guard<std::mutex> l(_mutex);

    auto & dst = _data[obj_id];
    for (auto & attrib : attribs)
      dst[attrib->id()] = std::move(attrib);
  }

private:
  std::mutex _mutex;
  std::vector<std::vector<std::unique_ptr<Attribute>>> _data;
};

TheWarehouse::TheWarehouse() : _store(std::make_unique<VecStore>()) {}
TheWarehouse::~TheWarehouse() {}

void isValid(MooseObject * obj);

void
TheWarehouse::add(std::shared_ptr<MooseObject> obj)
{
  isValid(obj.get());

  std::size_t obj_id = 0;
  {
    std::lock_guard<std::mutex> lock(_obj_mutex);
    _objects.push_back(obj);
    obj_id = _objects.size() - 1;
    _obj_ids[obj.get()] = obj_id;

    // reset/invalidate the query cache since query results may have been affected by this warehouse
    // insertion.
    _obj_cache.clear();
    _query_cache.clear();
  }

  std::vector<std::unique_ptr<Attribute>> attribs;
  readAttribs(obj.get(), attribs);
  _store->add(obj_id, std::move(attribs));
}

void
TheWarehouse::update(MooseObject * obj, const Attribute & extra)
{
  std::vector<std::unique_ptr<Attribute>> attribs;
  attribs.push_back(extra.clone());
  _store->set(_obj_ids[obj], std::move(attribs));
  // reset/invalidate the query cache since query results may have been affected by this object
  // attribute modification.
  _obj_cache.clear();
  _query_cache.clear();
}

void
TheWarehouse::update(MooseObject * obj)
{
  std::vector<std::unique_ptr<Attribute>> attribs;
  readAttribs(obj, attribs);
  _store->set(_obj_ids[obj], std::move(attribs));
  // reset/invalidate the query cache since query results may have been affected by this object
  // attribute modification.
  _obj_cache.clear();
  _query_cache.clear();
}

int
TheWarehouse::prepare(std::vector<std::unique_ptr<Attribute>> conds)
{
  bool sort = false;
  std::unique_ptr<Attribute> sorted_attrib;
  if (!conds.empty() && dynamic_cast<AttribSorted *>(conds.back().get()))
  {
    sorted_attrib = std::move(conds.back());
    static const AttribSorted sorted_attrib_true(*this, true);
    sort = sorted_attrib->isMatch(sorted_attrib_true);
    // Remove the sorted condition temporarily
    conds.pop_back();
  }

#ifdef DEBUG
  for (auto & cond : conds)
    mooseAssert(!dynamic_cast<AttribSorted *>(cond.get()),
                "There should be no sorted attributes in this container.");
#endif

  auto obj_ids = _store->query(conds);
  if (sorted_attrib)
    conds.push_back(std::move(sorted_attrib));

  std::lock_guard<std::mutex> lock(_obj_cache_mutex);
  _obj_cache.push_back({});
  auto query_id = _obj_cache.size() - 1;
  auto & vec = _obj_cache.back();
  {
    std::lock_guard<std::mutex> lock(_query_cache_mutex);
    _query_cache[std::move(conds)] = query_id;
  }

  std::lock_guard<std::mutex> o_lock(_obj_mutex);
  for (auto & id : obj_ids)
    vec.push_back(_objects[id].get());

  if (sort && !vec.empty() && dynamic_cast<DependencyResolverInterface *>(vec[0]))
  {
    std::vector<DependencyResolverInterface *> dependers;
    for (auto obj : vec)
    {
      auto d = dynamic_cast<DependencyResolverInterface *>(obj);
      if (!d)
      {
        dependers.clear();
        break;
      }
      dependers.push_back(d);
    }

    try
    {
      DependencyResolverInterface::sort(dependers);
    }
    catch (CyclicDependencyException<DependencyResolverInterface *> & e)
    {
      DependencyResolverInterface::cyclicDependencyError<UserObject *>(
          e, "Cyclic dependency detected in object ordering");
    }

    for (unsigned int i = 0; i < dependers.size(); i++)
      vec[i] = dynamic_cast<MooseObject *>(dependers[i]);
  }

  return query_id;
}

const std::vector<MooseObject *> &
TheWarehouse::query(int query_id)
{
  if (static_cast<std::size_t>(query_id) >= _obj_cache.size())
    throw std::runtime_error("unknown query id");
  return _obj_cache[query_id];
}

std::size_t
TheWarehouse::queryID(const std::vector<std::unique_ptr<Attribute>> & conds)
{
  {
    std::lock_guard<std::mutex> lock(_query_cache_mutex);
    auto it = _query_cache.find(conds);
    if (it != _query_cache.end())
      return it->second;
  }

  std::vector<std::unique_ptr<Attribute>> conds_clone;
  conds_clone.resize(conds.size());
  for (std::size_t i = 0; i < conds.size(); i++)
    conds_clone[i] = conds[i]->clone();
  return prepare(std::move(conds_clone));
}

std::size_t
TheWarehouse::count(const std::vector<std::unique_ptr<Attribute>> & conds)
{
  auto query_id = queryID(conds);
  std::lock_guard<std::mutex> lock(_obj_cache_mutex);
  auto & objs = query(query_id);
  std::size_t count = 0;
  for (auto obj : objs)
    if (obj->enabled())
      count++;
  return count;
}

void
TheWarehouse::readAttribs(const MooseObject * obj,
                          std::vector<std::unique_ptr<Attribute>> & attribs)
{
  for (auto & ref : _attrib_list)
  {
    attribs.emplace_back(ref->clone());
    attribs.back()->initFrom(obj);
  }
}

void
isValid(MooseObject * obj)
{
  auto blk = dynamic_cast<BlockRestrictable *>(obj);
  if (!blk)
    return;

  // Check variables
  auto c_ptr = dynamic_cast<Coupleable *>(obj);
  if (c_ptr)
    for (MooseVariableFEBase * var : c_ptr->getCoupledMooseVars())
      blk->checkVariable(*var);

  const InputParameters & parameters = obj->parameters();

  SubProblem & problem = *parameters.get<SubProblem *>("_subproblem");

  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  if (parameters.isParamValid("variable"))
  {
    // Try the scalar version first
    std::string variable_name = parameters.getMooseType("variable");
    if (variable_name == "")
      // When using vector variables, we are only going to use the first one in the list at the
      // interface level...
      variable_name = parameters.getVecMooseType("variable")[0];

    blk->checkVariable(problem.getVariable(
        tid, variable_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY));
  }
}
