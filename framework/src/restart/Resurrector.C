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
  : _fe_problem(fe_problem), _restartable(_fe_problem)
{
}

void
Resurrector::setRestartFile(const std::string & file_base)
{
  _restart_file_base = file_base;
}

void
Resurrector::restartFromFile()
{
  Moose::perf_log.push("restartFromFile()", "Setup");
  std::string file_name(_restart_file_base + ".xdr");
  MooseUtils::checkFileReadable(file_name);
  _restartable.readRestartableDataHeader(_restart_file_base + RESTARTABLE_DATA_EXT);
  _fe_problem.es().read(file_name,
                        DECODE,
                        EquationSystems::READ_DATA | EquationSystems::READ_ADDITIONAL_DATA,
                        _fe_problem.adaptivity().isOn());
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
