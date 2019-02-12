//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Attributes.h"

#include "TaggingInterface.h"
#include "BoundaryRestrictable.h"
#include "BlockRestrictable.h"
#include "SetupInterface.h"
#include "MooseVariableInterface.h"
#include "MooseVariableFE.h"
#include "ElementUserObject.h"
#include "SideUserObject.h"
#include "InternalSideUserObject.h"
#include "InterfaceUserObject.h"
#include "NodalUserObject.h"
#include "GeneralUserObject.h"
#include "ThreadedGeneralUserObject.h"
#include "ShapeUserObject.h"
#include "ShapeSideUserObject.h"
#include "ShapeElementUserObject.h"

bool
AttribTagBase::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribTagBase *>(&other);
  if (!a || a->_vals.size() < 1)
    return false;

  auto cond = a->_vals[0];
  for (auto val : _vals)
    if (val == cond)
      return true;
  return false;
}

bool
AttribTagBase::isEqual(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribTagBase *>(&other);
  if (!a || a->_vals.size() != _vals.size())
    return false;

  for (size_t i = 0; i < a->_vals.size(); i++)
    if (a->_vals[i] != _vals[i])
      return false;
  return true;
}

void
AttribMatrixTags::initFrom(const MooseObject * obj)
{
  _vals.clear();
  auto t = dynamic_cast<const TaggingInterface *>(obj);
  if (t)
  {
    for (auto & tag : t->getMatrixTags())
      _vals.push_back(static_cast<int>(tag));
  }
}

void
AttribVectorTags::initFrom(const MooseObject * obj)
{
  _vals.clear();
  auto t = dynamic_cast<const TaggingInterface *>(obj);
  if (t)
  {
    for (auto & tag : t->getVectorTags())
      _vals.push_back(static_cast<int>(tag));
  }
}

void
AttribExecOns::initFrom(const MooseObject * obj)
{
  _vals.clear();
  auto sup = dynamic_cast<const SetupInterface *>(obj);
  if (sup)
  {
    auto e = sup->getExecuteOnEnum();
    for (auto & on : e.items())
      if (e.contains(on))
        _vals.push_back(on);
  }
}

bool
AttribExecOns::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribExecOns *>(&other);
  if (!a || a->_vals.size() < 1)
    return false;
  auto cond = a->_vals[0];
  if (cond == Moose::ALL)
    return true;

  for (auto val : _vals)
    if (val == Moose::ALL || val == cond)
      return true;
  return false;
}

bool
AttribExecOns::isEqual(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribExecOns *>(&other);
  if (!a || a->_vals.size() != _vals.size())
    return false;

  for (size_t i = 0; i < a->_vals.size(); i++)
    if (a->_vals[i] != _vals[i])
      return false;
  return true;
}

void
AttribSubdomains::initFrom(const MooseObject * obj)
{
  _vals.clear();
  auto blk = dynamic_cast<const BlockRestrictable *>(obj);
  if (blk && blk->blockRestricted())
  {
    for (auto id : blk->blockIDs())
      _vals.push_back(id);
  }
  else
    _vals.push_back(Moose::ANY_BLOCK_ID);
}

bool
AttribSubdomains::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribSubdomains *>(&other);
  if (!a || a->_vals.size() < 1)
    return false;

  auto cond = a->_vals[0];
  if (cond == Moose::ANY_BLOCK_ID)
    return true;
  for (auto id : _vals)
  {
    if (id == cond || id == Moose::ANY_BLOCK_ID)
      return true;
  }
  return false;
}

bool
AttribSubdomains::isEqual(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribSubdomains *>(&other);
  if (!a || a->_vals.size() != _vals.size())
    return false;

  for (size_t i = 0; i < a->_vals.size(); i++)
    if (a->_vals[i] != _vals[i])
      return false;
  return true;
}

void
AttribBoundaries::initFrom(const MooseObject * obj)
{
  _vals.clear();
  auto bnd = dynamic_cast<const BoundaryRestrictable *>(obj);
  if (bnd && bnd->boundaryRestricted())
  {
    for (auto & bound : bnd->boundaryIDs())
      _vals.push_back(bound);
  }
  else
    _vals.push_back(Moose::ANY_BOUNDARY_ID);
}

bool
AttribBoundaries::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribBoundaries *>(&other);
  if (!a || a->_vals.size() < 1)
    return false;

  auto cond = a->_vals[0];
  if (cond == Moose::ANY_BOUNDARY_ID)
    return true;
  for (auto id : _vals)
  {
    if (id == cond || (!a->_must_be_restricted && (id == Moose::ANY_BOUNDARY_ID)))
      return true;
  }
  return false;
}

bool
AttribBoundaries::isEqual(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribBoundaries *>(&other);
  if (!a || a->_vals.size() != _vals.size())
    return false;

  for (size_t i = 0; i < a->_vals.size(); i++)
    if (a->_vals[i] != _vals[i])
      return false;
  return true;
}

void
AttribThread::initFrom(const MooseObject * obj)
{
  _val = obj->getParam<THREAD_ID>("_tid");
}
bool
AttribThread::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribThread *>(&other);
  return a && (a->_val == _val);
}
bool
AttribThread::isEqual(const Attribute & other) const
{
  return isMatch(other);
}

void
AttribPreIC::initFrom(const MooseObject * /*obj*/)
{
}
bool
AttribPreIC::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribPreIC *>(&other);
  return a && (a->_val == _val);
}

bool
AttribPreIC::isEqual(const Attribute & other) const
{
  return isMatch(other);
}

void
AttribPreAux::initFrom(const MooseObject * /*obj*/)
{
}
bool
AttribPreAux::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribPreAux *>(&other);
  return a && (a->_val == _val);
}

bool
AttribPreAux::isEqual(const Attribute & other) const
{
  return isMatch(other);
}

void
AttribName::initFrom(const MooseObject * obj)
{
  _val = obj->name();
}
bool
AttribName::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribName *>(&other);
  return a && (a->_val == _val);
}

bool
AttribName::isEqual(const Attribute & other) const
{
  return isMatch(other);
}

void
AttribSystem::initFrom(const MooseObject * /*obj*/)
{
}
bool
AttribSystem::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribSystem *>(&other);
  return a && (a->_val == _val);
}

bool
AttribSystem::isEqual(const Attribute & other) const
{
  return isMatch(other);
}

void
AttribVar::initFrom(const MooseObject * obj)
{
  auto vi = dynamic_cast<const MooseVariableInterface<Real> *>(obj);
  if (vi)
    _val = static_cast<int>(vi->mooseVariable()->number());
}
bool
AttribVar::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribVar *>(&other);
  return a && (a->_val == _val);
}

bool
AttribVar::isEqual(const Attribute & other) const
{
  return isMatch(other);
}

void
AttribInterfaces::initFrom(const MooseObject * obj)
{
  _val = 0;
  // clang-format off
  _val |= (unsigned int)Interfaces::UserObject                * (dynamic_cast<const UserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::ElementUserObject         * (dynamic_cast<const ElementUserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::SideUserObject            * (dynamic_cast<const SideUserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::InternalSideUserObject    * (dynamic_cast<const InternalSideUserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::InterfaceUserObject       * (dynamic_cast<const InterfaceUserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::NodalUserObject           * (dynamic_cast<const NodalUserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::GeneralUserObject         * (dynamic_cast<const GeneralUserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::ThreadedGeneralUserObject * (dynamic_cast<const ThreadedGeneralUserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::ShapeElementUserObject    * (dynamic_cast<const ShapeElementUserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::ShapeSideUserObject       * (dynamic_cast<const ShapeSideUserObject *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::Postprocessor             * (dynamic_cast<const Postprocessor *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::VectorPostprocessor       * (dynamic_cast<const VectorPostprocessor *>(obj) != nullptr);
  // clang-format on
}

bool
AttribInterfaces::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribInterfaces *>(&other);
  return a && (a->_val & _val);
}

bool
AttribInterfaces::isEqual(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribInterfaces *>(&other);
  return a && (a->_val == _val);
}
