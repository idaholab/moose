//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TheWarehouse.h"
#include "MooseObject.h"

#include "TaggingInterface.h"
#include "BoundaryRestrictable.h"
#include "BlockRestrictable.h"
#include "SetupInterface.h"
#include "MooseVariableInterface.h"
#include "MooseVariableFE.h"
#include "ElementUserObject.h"
#include "SideUserObject.h"
#include "InternalSideUserObject.h"
#include "NodalUserObject.h"
#include "GeneralUserObject.h"
#include "ThreadedGeneralUserObject.h"
#include "NonlocalKernel.h"
#include "NonlocalIntegratedBC.h"
#include "InternalSideIndicator.h"
#include "TransientMultiApp.h"
#include "MultiAppTransfer.h"
#include "ShapeUserObject.h"
#include "ShapeSideUserObject.h"
#include "ShapeElementUserObject.h"

#include <memory>
#include <mutex>

Interfaces
operator|(Interfaces l, Interfaces r)
{
  return static_cast<Interfaces>(static_cast<unsigned int>(l) | static_cast<unsigned int>(r));
}

std::string
printAttribs(const std::vector<Attribute> & attribs)
{
  std::string s = "attrib_set:\n";
  for (auto & attrib : attribs)
  {
    s += "    ";
    switch (attrib.id)
    {
      case AttributeId::Thread:
        s += "Thread=";
        s += std::to_string(attrib.value);
        break;
      case AttributeId::Name:
        s += "Name=";
        s += attrib.strvalue;
        break;
      case AttributeId::System:
        s += "System=";
        s += attrib.strvalue;
        break;
      case AttributeId::Variable:
        s += "Variable=";
        s += std::to_string(attrib.value);
        break;
      case AttributeId::Interfaces:
        s += "Interfaces=";
        s += std::to_string(attrib.value);
        break;
      case AttributeId::PreIC:
        s += "PreIC=";
        s += std::to_string(attrib.value);
        break;
      case AttributeId::PreAux:
        s += "PreAux=";
        s += std::to_string(attrib.value);
        break;
      case AttributeId::Boundary:
        s += "Boundary=";
        s += std::to_string(attrib.value);
        break;
      case AttributeId::Subdomain:
        s += "Subdomain=";
        s += std::to_string(attrib.value);
        break;
      case AttributeId::ExecOn:
        s += "ExecOn=";
        s += std::to_string(attrib.value);
        break;
      case AttributeId::VectorTag:
        s += "VectorTag=";
        s += std::to_string(attrib.value);
        break;
      case AttributeId::MatrixTag:
        s += "MatrixTag=";
        s += std::to_string(attrib.value);
        break;
      default:
        throw std::runtime_error("unknown AttributeId " +
                                 std::to_string(static_cast<int>(attrib.id)));
    }
    s += "\n";
  }
  return s;
}

class Storage
{
public:
  virtual ~Storage() = default;

  virtual void add(size_t obj_id, const std::vector<Attribute> & attribs) = 0;
  virtual std::vector<size_t> query(const std::vector<Attribute> & conds) = 0;
  virtual void set(size_t obj_id, const std::vector<Attribute> & attribs) = 0;
};

class VecStore : public Storage
{
private:
  struct Data
  {
    size_t id;
    std::string name;
    std::string system;
    int thread = 0;
    int variable = -1;
    int64_t interfaces = 0; // this is a bitmask for Interfaces enum
    std::vector<boundary_id_type> boundaries;
    std::vector<subdomain_id_type> subdomains;
    std::vector<int> execute_ons;
    std::vector<int> vector_tags;
    std::vector<int> matrix_tags;
    // TODO: delete these two later - they are temporary hacks for dealing with inter-system
    // dependencies:
    bool pre_ic = false;
    bool pre_aux = false;
  };

public:
  virtual void add(size_t obj_id, const std::vector<Attribute> & attribs) override
  {
    std::lock_guard<std::mutex> l(_mutex);
    if (obj_id < _data.size())
      throw std::runtime_error("object with id " + std::to_string(obj_id) + " already added");

    _data.push_back({});
    auto & d = _data.back();
    d.id = obj_id;
    set(d, attribs);
  }

  virtual std::vector<size_t> query(const std::vector<Attribute> & conds) override
  {
    std::vector<size_t> objs;
    for (size_t i = 0; i < _data.size(); i++)
    {
      Data * d = nullptr;
      {
        std::lock_guard<std::mutex> l(_mutex);
        d = &_data[i];
      }

      bool passes = true;
      for (auto & cond : conds)
      {
        switch (cond.id)
        {
          case AttributeId::Thread:
            passes = cond.value == d->thread;
            break;
          case AttributeId::Name:
            passes = cond.strvalue == d->name;
            break;
          case AttributeId::System:
            passes = cond.strvalue == d->system;
            break;
          case AttributeId::Variable:
            passes = cond.value == d->variable;
            break;
          case AttributeId::Interfaces:
            passes = static_cast<unsigned int>(cond.value) &
                     static_cast<unsigned int>(d->interfaces); // check bit in bitmask
            break;
          // TODO: delete this case later - it is a temporary hack for dealing with inter-system
          // dependencies:
          case AttributeId::PreIC:
            passes = cond.value == d->pre_ic;
            break;
          // TODO: delete this case later - it is a temporary hack for dealing with inter-system
          // dependencies:
          case AttributeId::PreAux:
            passes = cond.value == d->pre_aux;
            break;
          case AttributeId::Boundary:
            passes = false;
            for (auto val : d->boundaries)
              if (cond.value == Moose::ANY_BOUNDARY_ID || val == Moose::ANY_BOUNDARY_ID ||
                  cond.value == val)
              {
                passes = true;
                break;
              }
            break;
          case AttributeId::Subdomain:
            passes = false;
            for (auto val : d->subdomains)
              if (cond.value == Moose::ANY_BLOCK_ID || val == Moose::ANY_BLOCK_ID ||
                  cond.value == val)
              {
                passes = true;
                break;
              }
            break;
          case AttributeId::ExecOn:
            passes = false;
            for (auto val : d->execute_ons)
              if (cond.value == val)
              {
                passes = true;
                break;
              }
            break;
          case AttributeId::VectorTag:
            passes = false;
            for (auto val : d->vector_tags)
              if (cond.value == val)
              {
                passes = true;
                break;
              }
            break;
          case AttributeId::MatrixTag:
            passes = false;
            for (auto val : d->matrix_tags)
              if (cond.value == val)
              {
                passes = true;
                break;
              }
            break;
          default:
            throw std::runtime_error("unknown AttributeId " +
                                     std::to_string(static_cast<int>(cond.id)));
        }
        if (!passes)
          break;
      }
      if (passes)
        objs.push_back(i);
    }
    return objs;
  }

  virtual void set(size_t obj_id, const std::vector<Attribute> & attribs) override
  {
    std::lock_guard<std::mutex> l(_mutex);
    Data * dat = nullptr;
    if (_data[obj_id].id == obj_id)
      dat = &_data[obj_id];
    else
      for (auto & d : _data)
        if (d.id == obj_id)
        {
          dat = &d;
          break;
        }

    if (!dat)
      throw std::runtime_error("unknown object id " + std::to_string(obj_id));

    set(*dat, attribs);
  }

private:
  void set(Data & d, const std::vector<Attribute> & attribs)
  {
    for (auto & attrib : attribs)
    {
      switch (attrib.id)
      {
        case AttributeId::Thread:
          d.thread = attrib.value;
          break;
        case AttributeId::Name:
          d.name = attrib.strvalue;
          break;
        case AttributeId::System:
          d.system = attrib.strvalue;
          break;
        case AttributeId::Variable:
          d.variable = attrib.value;
          break;
        case AttributeId::Interfaces:
          d.interfaces = attrib.value;
          break;
        // TODO: delete this case later - it is a temporary hack for dealing with inter-system
        // dependencies:
        case AttributeId::PreIC:
          d.pre_ic = attrib.value;
          break;
        // TODO: delete this case later - it is a temporary hack for dealing with inter-system
        // dependencies:
        case AttributeId::PreAux:
          d.pre_aux = attrib.value;
          break;
        case AttributeId::Boundary:
          d.boundaries.push_back(attrib.value);
          break;
        case AttributeId::Subdomain:
          d.subdomains.push_back(attrib.value);
          break;
        case AttributeId::ExecOn:
          d.execute_ons.push_back(attrib.value);
          break;
        case AttributeId::VectorTag:
          d.vector_tags.push_back(attrib.value);
          break;
        case AttributeId::MatrixTag:
          d.matrix_tags.push_back(attrib.value);
          break;
        default:
          throw std::runtime_error("unknown AttributeId " +
                                   std::to_string(static_cast<int>(attrib.id)));
      }
    }
  }

  std::mutex _mutex;
  std::vector<Data> _data;
};

TheWarehouse::TheWarehouse() : _store(new VecStore()) {}
TheWarehouse::~TheWarehouse() {}

static std::mutex obj_mutex;
static std::mutex cache_mutex;

void isValid(MooseObject * obj);

void
TheWarehouse::add(std::shared_ptr<MooseObject> obj, const std::string & system)
{
  isValid(obj.get());

  std::vector<Attribute> attribs;
  readAttribs(obj.get(), system, attribs);
  size_t obj_id = 0;
  {
    std::lock_guard<std::mutex> lock(obj_mutex);
    _objects.push_back(obj);
    obj_id = _objects.size() - 1;
    _obj_ids[obj.get()] = obj_id;
  }
  _store->add(obj_id, attribs);

  _obj_cache.clear();
  _query_cache.clear();
}

void
TheWarehouse::update(MooseObject * obj, const std::vector<Attribute> & extras /*={}*/)
{
  std::vector<Attribute> attribs;
  readAttribs(obj, "", attribs);
  attribs.insert(attribs.end(), extras.begin(), extras.end());
  _store->set(_obj_ids[obj], attribs);

  _obj_cache.clear();
  _query_cache.clear();
}

int
TheWarehouse::prepare(const std::vector<Attribute> & conds)
{
  auto obj_ids = _store->query(conds);

  std::lock_guard<std::mutex> c_lock(cache_mutex);
  _obj_cache.push_back({});
  auto query_id = _obj_cache.size() - 1;
  auto & vec = _obj_cache.back();
  _query_cache[conds] = query_id;

  std::lock_guard<std::mutex> o_lock(obj_mutex);
  for (auto & id : obj_ids)
    vec.push_back(_objects[id].get());

  if (!vec.empty() && dynamic_cast<DependencyResolverInterface *>(vec[0]))
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
    catch (CyclicDependencyException<GeneralUserObject *> & e)
    {
      DependencyResolverInterface::cyclicDependencyError<GeneralUserObject *>(
          e, "Cyclic dependency detected in object ordering");
    }

    for (unsigned int i = 0; i < dependers.size(); i++)
      vec[i] = dynamic_cast<MooseObject *>(dependers[i]);
  }

  return query_id;
}

const std::vector<MooseObject *>
TheWarehouse::query(int query_id)
{
  if (static_cast<size_t>(query_id) >= _obj_cache.size())
    throw std::runtime_error("unknown query id");
  std::lock_guard<std::mutex> lock(cache_mutex);
  return _obj_cache[query_id];
}

size_t
TheWarehouse::count(const std::vector<Attribute> & conds)
{
  auto query_id = prepare(conds);
  if (static_cast<size_t>(query_id) >= _obj_cache.size())
    throw std::runtime_error("unknown query id");
  std::lock_guard<std::mutex> lock(cache_mutex);
  auto & objs = _obj_cache[query_id];
  size_t count = 0;
  for (auto obj : objs)
    if (obj->enabled())
      count++;
  return count;
}

void
TheWarehouse::readAttribs(const MooseObject * obj,
                          const std::string & system,
                          std::vector<Attribute> & attribs)
{
  if (!system.empty())
    attribs.push_back({AttributeId::System, 0, system});
  attribs.push_back({AttributeId::Name, 0, obj->name()});
  attribs.push_back({AttributeId::Thread, static_cast<int>(obj->getParam<THREAD_ID>("_tid")), ""});

  // clang-format off
  unsigned int imask = 0;
  imask |= (unsigned int)Interfaces::UserObject                * (dynamic_cast<const UserObject *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::ElementUserObject         * (dynamic_cast<const ElementUserObject *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::SideUserObject            * (dynamic_cast<const SideUserObject *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::InternalSideUserObject    * (dynamic_cast<const InternalSideUserObject *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::NodalUserObject           * (dynamic_cast<const NodalUserObject *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::GeneralUserObject         * (dynamic_cast<const GeneralUserObject *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::ThreadedGeneralUserObject * (dynamic_cast<const ThreadedGeneralUserObject *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::ShapeElementUserObject    * (dynamic_cast<const ShapeElementUserObject *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::ShapeSideUserObject       * (dynamic_cast<const ShapeSideUserObject *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::Postprocessor             * (dynamic_cast<const Postprocessor *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::VectorPostprocessor       * (dynamic_cast<const VectorPostprocessor *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::NonlocalKernel            * (dynamic_cast<const NonlocalKernel *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::NonlocalIntegratedBC      * (dynamic_cast<const NonlocalIntegratedBC *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::InternalSideIndicator     * (dynamic_cast<const InternalSideIndicator *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::TransientMultiApp         * (dynamic_cast<const TransientMultiApp *>(obj) != nullptr);
  imask |= (unsigned int)Interfaces::MultiAppTransfer          * (dynamic_cast<const MultiAppTransfer *>(obj) != nullptr);
  attribs.push_back({AttributeId::Interfaces, static_cast<int>(imask), ""});
  // clang-format on

  auto vi = dynamic_cast<const MooseVariableInterface<Real> *>(obj);
  if (vi)
    attribs.push_back({AttributeId::Variable, static_cast<int>(vi->mooseVariable()->number()), ""});

  auto ti = dynamic_cast<const TaggingInterface *>(obj);
  if (ti)
  {
    for (auto tag : ti->getVectorTags())
      attribs.push_back({AttributeId::VectorTag, static_cast<int>(tag), ""});
    for (auto tag : ti->getMatrixTags())
      attribs.push_back({AttributeId::MatrixTag, static_cast<int>(tag), ""});
  }
  auto blk = dynamic_cast<const BlockRestrictable *>(obj);
  if (blk)
  {
    if (blk->blockRestricted())
      for (auto id : blk->blockIDs())
        attribs.push_back({AttributeId::Subdomain, id, ""});
    else
      attribs.push_back({AttributeId::Subdomain, Moose::ANY_BLOCK_ID, ""});
  }
  auto bnd = dynamic_cast<const BoundaryRestrictable *>(obj);
  if (bnd && bnd->boundaryRestricted())
  {
    if (bnd->boundaryRestricted())
      for (auto & bound : bnd->boundaryIDs())
        attribs.push_back({AttributeId::Boundary, bound, ""});
    else
      attribs.push_back({AttributeId::Boundary, Moose::ANY_BOUNDARY_ID, ""});
  }
  auto sup = dynamic_cast<const SetupInterface *>(obj);
  if (sup)
  {
    auto e = sup->getExecuteOnEnum();
    for (auto & on : e.items())
    {
      if (e.contains(on))
        attribs.push_back({AttributeId::ExecOn, on, ""});
    }
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
