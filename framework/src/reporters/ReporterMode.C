//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterMode.h"
const ReporterMode REPORTER_MODE_UNSET("UNSET");
const ReporterMode REPORTER_MODE_ROOT("ROOT");
const ReporterMode REPORTER_MODE_REPLICATED("REPLICATED");
const ReporterMode REPORTER_MODE_DISTRIBUTED("DISTRIBUTED");

ReporterMode::ReporterMode(const std::string & key) : MooseEnumItem(key, ID_COUNTER++) {}

int ReporterMode::ID_COUNTER = 0;

ReporterProducerEnum::ReporterProducerEnum() : MooseEnum() {}

void
ReporterProducerEnum::insert(const ReporterMode & mode)
{
  addEnumerationItem(mode);
}

void
ReporterProducerEnum::clear()
{
  _items.clear();
}
