//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseBase.h"

#include "MooseError.h"
#include "InputParameters.h"
#include "MooseApp.h"

MooseBase::MooseBase(const std::string & type,
                     const std::string & name,
                     MooseApp & app,
                     const InputParameters & params)
  : _app(app), _type(type), _name(name), _params(params)
{
  // So that all errors that happen within InputParameters can have
  // the added context of this object, and also stream to the correct
  // console with the correct multiapp prefix (if any)
  const_cast<InputParameters &>(params).set<const MooseBase *>("_moose_base_ptr") = this;
}

std::string
MooseBase::typeAndName() const
{
  return type() + std::string(" \"") + name() + std::string("\"");
}

[[noreturn]] void
MooseBase::callMooseError(std::string msg,
                          const bool with_prefix,
                          const hit::Node * node /* = nullptr */) const
{
  if (!node)
    node = _params.getHitNode();

  _app.getOutputWarehouse().mooseConsole();
  const std::string multiapp_prefix = _app.isUltimateMaster() ? "" : _app.name();
  if (with_prefix)
    msg = messagePrefix() + "\n\n" + msg;
  moose::internal::mooseErrorRaw(msg, multiapp_prefix, node);
}

std::string
MooseBase::messagePrefix() const
{
  std::string prefix = "The following occurred in the ";
  const std::string base = _params.getBase() ? *_params.getBase() : "object";
  prefix += base;
  if (base != name())
    prefix += " '" + name() + "'";
  prefix += " of type " + type() + ".";
  return prefix;
}
