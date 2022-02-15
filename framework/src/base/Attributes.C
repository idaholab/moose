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
#include "Reporter.h"
#include "SystemBase.h"

#include <algorithm>

std::ostream &
operator<<(std::ostream & os, Interfaces & iface)
{
  os << "Interfaces(";
  if (static_cast<bool>(iface & Interfaces::UserObject))
    os << "|UserObject";
  if (static_cast<bool>(iface & Interfaces::ElementUserObject))
    os << "|ElementUserObject";
  if (static_cast<bool>(iface & Interfaces::SideUserObject))
    os << "|SideUserObject";
  if (static_cast<bool>(iface & Interfaces::InternalSideUserObject))
    os << "|InternalSideUserObject";
  if (static_cast<bool>(iface & Interfaces::NodalUserObject))
    os << "|NodalUserObject";
  if (static_cast<bool>(iface & Interfaces::GeneralUserObject))
    os << "|GeneralUserObject";
  if (static_cast<bool>(iface & Interfaces::ThreadedGeneralUserObject))
    os << "|ThreadedGeneralUserObject";
  if (static_cast<bool>(iface & Interfaces::ShapeElementUserObject))
    os << "|ShapeElementUserObject";
  if (static_cast<bool>(iface & Interfaces::ShapeSideUserObject))
    os << "|ShapeSideUserObject";
  if (static_cast<bool>(iface & Interfaces::Postprocessor))
    os << "|Postprocessor";
  if (static_cast<bool>(iface & Interfaces::VectorPostprocessor))
    os << "|VectorPostprocessor";
  if (static_cast<bool>(iface & Interfaces::InterfaceUserObject))
    os << "|InterfaceUserObject";
  if (static_cast<bool>(iface & Interfaces::Reporter))
    os << "|Reporter";
  os << ")";
  return os;
}

bool
AttribTagBase::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribTagBase *>(&other);
  if (!a)
    return false;
  if (a->_vals.size() == 0)
    return true; // the condition is empty tags - which we take to mean any tag should match

  // return true if any single tag matches between the two attribute objects
  for (auto val : _vals)
    if (std::find(a->_vals.begin(), a->_vals.end(), val) != a->_vals.end())
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
  else if (cond == Moose::INVALID_BLOCK_ID)
    return false;

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

  // return true if a single tag matches between the two attribute objects
  for (auto val : _vals)
  {
    if (!a->_must_be_restricted && (val == Moose::ANY_BOUNDARY_ID))
      return true;
    if (std::find(a->_vals.begin(), a->_vals.end(), val) != a->_vals.end())
      return true;
    else if (std::find(a->_vals.begin(), a->_vals.end(), Moose::ANY_BOUNDARY_ID) != a->_vals.end())
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
AttribSysNum::initFrom(const MooseObject * obj)
{
  auto * sys = obj->getParam<SystemBase *>("_sys");

  if (sys)
    _val = sys->number();
}

bool
AttribSysNum::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribSysNum *>(&other);
  return a && (a->_val == _val);
}

bool
AttribSysNum::isEqual(const Attribute & other) const
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

bool
AttribPreAux::isMatch(const Attribute & other) const
{
  const auto a = dynamic_cast<const AttribPreAux *>(&other);

  bool is_match = false;

  if (a && !_vals.empty() && !a->_vals.empty())
  {
    is_match = std::includes(_vals.begin(), _vals.end(), a->_vals.begin(), a->_vals.end()) ||
               std::includes(a->_vals.begin(), a->_vals.end(), _vals.begin(), _vals.end());
  }

  return is_match;
}

bool
AttribPreAux::isEqual(const Attribute & other) const
{
  const auto a = dynamic_cast<const AttribPreAux *>(&other);
  return a && a->_vals == _vals;
}

void
AttribPreAux::initFrom(const MooseObject * /*obj*/)
{
}

bool
AttribPostAux::isMatch(const Attribute & other) const
{
  const auto a = dynamic_cast<const AttribPostAux *>(&other);

  bool is_match = false;

  if (a && !_vals.empty() && !a->_vals.empty())
  {
    is_match = std::includes(_vals.begin(), _vals.end(), a->_vals.begin(), a->_vals.end()) ||
               std::includes(a->_vals.begin(), a->_vals.end(), _vals.begin(), _vals.end());
  }

  return is_match;
}

bool
AttribPostAux::isEqual(const Attribute & other) const
{
  const auto a = dynamic_cast<const AttribPostAux *>(&other);
  return a && a->_vals == _vals;
}

void
AttribPostAux::initFrom(const MooseObject * /*obj*/)
{
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
AttribSystem::initFrom(const MooseObject * obj)
{
  if (!obj->isParamValid("_moose_warehouse_system_name"))
    mooseError("The base objects supplied to the TheWarehouse must call "
               "'registerSystemAttributeName' method in the validParams function.");
  _val = obj->getParam<std::string>("_moose_warehouse_system_name");
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
AttribResidualObject::initFrom(const MooseObject * obj)
{
  _val = obj->getParam<bool>("_residual_object");
  _initd = true;
}

bool
AttribResidualObject::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribResidualObject *>(&other);
  return _initd && a && a->_initd && (a->_val == _val);
}

bool
AttribResidualObject::isEqual(const Attribute & other) const
{
  return isMatch(other);
}

void
AttribVar::initFrom(const MooseObject * obj)
{
  auto vi = dynamic_cast<const MooseVariableInterface<Real> *>(obj);
  if (vi)
    _val = static_cast<int>(vi->mooseVariableBase()->number());
}
bool
AttribVar::isMatch(const Attribute & other) const
{
  auto a = dynamic_cast<const AttribVar *>(&other);
  return a && (_val != -1) && (a->_val == _val);
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
  _val |= (unsigned int)Interfaces::BlockRestrictable         * (dynamic_cast<const BlockRestrictable *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::BoundaryRestrictable      * (dynamic_cast<const BoundaryRestrictable *>(obj) != nullptr);
  _val |= (unsigned int)Interfaces::Reporter                  * (dynamic_cast<const Reporter *>(obj) != nullptr);
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
