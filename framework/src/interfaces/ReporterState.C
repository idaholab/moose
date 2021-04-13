//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterState.h"

#include "ReporterData.h"
#include "MooseObject.h"

ReporterStateBase::ReporterStateBase(const ReporterName & name)
  : _reporter_name(name), _producer(nullptr)
{
}

void
ReporterStateBase::addConsumer(ReporterMode mode, const MooseObject & moose_object)
{
  _consumers.emplace(mode, &moose_object);
}

void
ReporterStateBase::setProducer(const MooseObject & moose_object)
{
  mooseAssert(!_producer, "Producer already set");
  _producer = &moose_object;
}

std::string
ReporterStateBase::getInfo(const ReporterContextBase * context /* = nullptr */) const
{
  std::stringstream oss;

  if (getReporterName().isPostprocessor())
    oss << "Postprocessor \"" << getReporterName().getObjectName() << "\":\n";
  else
  {
    oss << getReporterName().specialTypeToName() << " \"" << getReporterName().getCombinedName()
        << "\":\n  Type:\n    " << valueType() << "\n";
  }
  oss << "  Producer:\n    ";
  if (!hasProducer())
    oss << "None";
  else
  {
    oss << getProducer().type() << " \"" << getProducer().name() << "\"";
    if (context)
      oss << "\n  Context type:\n    " << context->contextType();
  }
  oss << "\n  Consumer(s):\n";
  if (getConsumers().empty())
    oss << "    None\n";
  else
    for (const auto & mode_object_pair : getConsumers())
    {
      const ReporterMode mode = mode_object_pair.first;
      const MooseObject * object = mode_object_pair.second;
      oss << "    " << object->typeAndName() << " (mode: " << mode << ")\n";
    }

  return oss.str();
}

bool
operator<(const std::pair<ReporterMode, const MooseObject *> & a,
          const std::pair<ReporterMode, const MooseObject *> & b)
{
  // Sort by object type, object name, and then mode
  return a.second->type() < b.second->type() ||
         (a.second->type() == b.second->type() && a.second->name() < b.second->name()) ||
         (a.second->type() == b.second->type() && a.second->name() == b.second->name() &&
          a.first < b.first);
}
