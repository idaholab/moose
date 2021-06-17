//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterContext.h"

#include "ReporterData.h"

ReporterContextBase::ReporterContextBase(const libMesh::ParallelObject & other,
                                         const MooseObject & producer)
  : libMesh::ParallelObject(other), _producer(producer)
{
}

void
ReporterContextBase::init(const ReporterMode & mode)
{
  if (mode != REPORTER_MODE_UNSET)
    _producer_enum.assign(mode);
}

const ReporterProducerEnum &
ReporterContextBase::getProducerModeEnum() const
{
  return _producer_enum;
}

void
ReporterContextBase::requiresConsumerModes(const ReporterStateBase & state,
                                           const std::set<ReporterMode> & modes) const
{
  for (const auto & mode_object_pair : state.getConsumers())
    if (!modes.count(mode_object_pair.first))
    {
      std::stringstream oss;
      std::copy(modes.begin(), modes.end(), std::ostream_iterator<ReporterMode>(oss, " "));

      mooseError("The Reporter value '",
                 name(),
                 "' is being produced in ",
                 _producer_enum,
                 " mode, but ",
                 mode_object_pair.second->typeAndName(),
                 " is requesting to consume it in ",
                 mode_object_pair.first,
                 " mode, which is not supported.\n\nThe mode must be { ",
                 oss.str(),
                 " }.\n\n",
                 ReporterData::getReporterInfo(state, this));
    }
}
