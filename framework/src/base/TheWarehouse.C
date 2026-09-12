//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "ScalarCoupleable.h"
#include "ScalarKernelBase.h"
#include "MooseVariableScalar.h"
#include "Conversion.h"

#include <algorithm>
#include <iterator>
#include <memory>

class WarehouseStorage
{
public:
  virtual ~WarehouseStorage() = default;
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

class VecStore : public WarehouseStorage
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
      {
        mooseAssert(std::find(ids.begin(), ids.end(), i) == ids.end(), "Duplicate object");
        ids.push_back(i);
      }
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

    mooseAssert(!_obj_ids.count(obj.get()), obj->typeAndName() + " has already been added");

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
  auto & vec = _obj_cache.emplace_back(obj_ids.size());
  const auto query_id = _obj_cache.size() - 1;
  {
    std::lock_guard<std::mutex> lock(_query_cache_mutex);
    _query_cache[std::move(conds)] = query_id;
  }

  std::lock_guard<std::mutex> o_lock(_obj_mutex);
  for (const auto i : index_range(obj_ids))
  {
    auto obj = _objects[obj_ids[i]].get();
    mooseAssert(std::find(vec.begin(), vec.end(), obj) == vec.end(), "Duplicate object");
    vec[i] = obj;
  }

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
      DependencyResolverInterface::cyclicDependencyError<MooseObject *>(
          e,
          "Cyclic dependency detected in object ordering",
          [](DependencyResolverInterface * obj)
          {
            auto * moose_obj = dynamic_cast<MooseObject *>(obj);
            mooseAssert(moose_obj, "Failed to cast dependency object to MooseObject");
            return moose_obj->name();
          });
    }

    mooseAssert(dependers.size() == vec.size(), "Dependency resolution size mismatch");
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

/**
 * Check the scalar variables coupled into a scalar kernel. The kernel assembles the matrix entries
 * pairing its own variable's dofs with the coupled variable's dofs, and those entries are
 * preallocated only on the elements where both variables exist. A scalar kernel carries no block
 * restriction of its own to compare against, so the condition is a relation between the two
 * variables: they must share at least one block.
 */
void
checkCoupledScalarVariableBlocks(ScalarKernelBase & scalar_kernel)
{
  const auto & var = scalar_kernel.variable();
  const auto & subdomains = var.activeSubdomains();

  // An empty set of active subdomains means the variable lives on the whole mesh
  if (subdomains.empty())
    return;

  for (const MooseVariableScalar * const coupled_var : scalar_kernel.getCoupledMooseScalarVars())
  {
    const auto & coupled_subdomains = coupled_var->activeSubdomains();
    if (coupled_subdomains.empty())
      continue;

    std::set<SubdomainID> shared_subdomains;
    std::set_intersection(subdomains.begin(),
                          subdomains.end(),
                          coupled_subdomains.begin(),
                          coupled_subdomains.end(),
                          std::inserter(shared_subdomains, shared_subdomains.begin()));

    if (shared_subdomains.empty())
      mooseError("The 'block' parameter of the scalar variable '",
                 coupled_var->name(),
                 "' coupled into the object '",
                 scalar_kernel.name(),
                 "' must overlap the 'block' parameter of the variable '",
                 var.name(),
                 "' that the object acts on:\n    Variable '",
                 var.name(),
                 "': ",
                 Moose::stringify(subdomains, ", "),
                 "\n    Variable '",
                 coupled_var->name(),
                 "': ",
                 Moose::stringify(coupled_subdomains, ", "));
  }
}

void
isValid(MooseObject * obj)
{
  auto blk = dynamic_cast<BlockRestrictable *>(obj);
  if (!blk)
  {
    // Scalar kernels are not block restrictable, so their coupled scalar variables are checked
    // against the variable the kernel acts on
    if (auto scalar_kernel = dynamic_cast<ScalarKernelBase *>(obj))
      checkCoupledScalarVariableBlocks(*scalar_kernel);
    return;
  }

  // Check variables
  auto c_ptr = dynamic_cast<Coupleable *>(obj);
  if (c_ptr)
    for (MooseVariableFEBase * var : c_ptr->getCoupledMooseVars())
      blk->checkVariable(*var);

  // Check scalar variables
  auto sc_ptr = dynamic_cast<ScalarCoupleable *>(obj);
  if (sc_ptr)
    for (MooseVariableScalar * var : sc_ptr->getCoupledMooseScalarVars())
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
