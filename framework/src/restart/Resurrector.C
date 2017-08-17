/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "Resurrector.h"
#include "FEProblem.h"
#include "MooseUtils.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"

#include <stdio.h>
#include <sys/stat.h>

const std::string Resurrector::MAT_PROP_EXT(".msmp");
const std::string Resurrector::RESTARTABLE_DATA_EXT(".rd");

Resurrector::Resurrector(FEProblemBase & fe_problem)
  : _fe_problem(fe_problem), _restart_file_suffix("xdr"), _restartable(_fe_problem)
{
}

void
Resurrector::setRestartFile(const std::string & file_base)
{
  _restart_file_base = file_base;
}

void
Resurrector::setRestartSuffix(const std::string & file_ext)
{
  _restart_file_suffix = file_ext;
}

void
Resurrector::restartFromFile()
{
  Moose::perf_log.push("restartFromFile()", "Setup");
  std::string file_name(_restart_file_base + '.' + _restart_file_suffix);
  MooseUtils::checkFileReadable(file_name);
  _restartable.readRestartableDataHeader(_restart_file_base + RESTARTABLE_DATA_EXT);
  unsigned int read_flags = EquationSystems::READ_DATA;
  if (!_fe_problem.skipAdditionalRestartData())
    read_flags |= EquationSystems::READ_ADDITIONAL_DATA;

  // Set libHilbert renumbering flag to false.  We don't support
  // N-to-M restarts regardless, and if we're *never* going to do
  // N-to-M restarts then libHilbert is just unnecessary computation
  // and communication.
  const bool renumber = false;

  // DECODE or READ based on suffix.
  // MOOSE doesn't currently use partition-agnostic renumbering, since
  // it can break restarts when multiple nodes are at the same point
  _fe_problem.es().read(file_name, read_flags, renumber);

  _fe_problem.getNonlinearSystemBase().update();
  Moose::perf_log.pop("restartFromFile()", "Setup");
}

void
Resurrector::restartRestartableData()
{
  Moose::perf_log.push("restartRestartableData()", "Setup");
  _restartable.readRestartableData(_fe_problem.getMooseApp().getRestartableData(),
                                   _fe_problem.getMooseApp().getRecoverableData());
  Moose::perf_log.pop("restartRestartableData()", "Setup");
}
