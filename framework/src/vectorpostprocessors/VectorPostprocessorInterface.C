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

#include "VectorPostprocessorInterface.h"
#include "FEProblem.h"
#include "VectorPostprocessor.h"
#include "MooseTypes.h"

VectorPostprocessorInterface::VectorPostprocessorInterface(const MooseObject * moose_object)
  : _vpi_params(moose_object->parameters()),
    // TODO: Retrieve using checked pointer method
    _vpi_feproblem(*_vpi_params.get<FEProblemBase *>("_fe_problem_base")),
    _vpi_tid(_vpi_params.have_parameter<THREAD_ID>("_tid") ? _vpi_params.get<THREAD_ID>("_tid") : 0)
{
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValue(const std::string & name,
                                                          const std::string & vector_name)
{
  return _vpi_feproblem.getVectorPostprocessorValue(_vpi_params.get<VectorPostprocessorName>(name),
                                                    vector_name);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueByName(
    const VectorPostprocessorName & name, const std::string & vector_name)
{
  return _vpi_feproblem.getVectorPostprocessorValue(name, vector_name);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOld(const std::string & name,
                                                             const std::string & vector_name)
{
  return _vpi_feproblem.getVectorPostprocessorValueOld(
      _vpi_params.get<VectorPostprocessorName>(name), vector_name);
}

const VectorPostprocessorValue &
VectorPostprocessorInterface::getVectorPostprocessorValueOldByName(
    const VectorPostprocessorName & name, const std::string & vector_name)
{
  return _vpi_feproblem.getVectorPostprocessorValueOld(name, vector_name);
}

bool
VectorPostprocessorInterface::hasVectorPostprocessor(const std::string & name) const
{
  return _vpi_feproblem.hasVectorPostprocessor(_vpi_params.get<VectorPostprocessorName>(name));
}

bool
VectorPostprocessorInterface::hasVectorPostprocessorByName(
    const VectorPostprocessorName & name) const
{
  return _vpi_feproblem.hasVectorPostprocessor(name);
}
