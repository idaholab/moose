//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterState.h"

#include "MooseObject.h"

ReporterStateBase::ReporterStateBase(const ReporterName & name) : _reporter_name(name) {}

void
ReporterStateBase::addConsumer(ReporterMode mode, const MooseObject & moose_object)
{
  _consumers.emplace(mode, &moose_object);
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
