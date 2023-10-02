//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LateRestartableDataRestorer.h"

#include "RestartableDataReader.h"

LateRestartableDataRestorer::LateRestartableDataRestorer(RestartableDataReader & reader)
  : _reader(reader)
{
}

bool
LateRestartableDataRestorer::isRestorable(const std::string & name,
                                          const std::type_info & type,
                                          const THREAD_ID tid /* = 0 */) const
{
  return _reader.isLateRestorable(name, type, tid);
}

void
LateRestartableDataRestorer::restore(const std::string & name, const THREAD_ID tid /* = 0 */)
{
  _reader.restoreLateData(name, tid, {});
}
