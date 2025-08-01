//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UseOldVectorPostprocessor.h"

// MOOSE includes
#include "MooseVariable.h"
#include "ThreadedElementLoopBase.h"
#include "ThreadedNodeLoop.h"

registerMooseObject("MooseTestApp", UseOldVectorPostprocessor);

InputParameters
UseOldVectorPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("A test Postprocessor that couples to old values of a "
                             "VectorPostprocessor and verifies that the the current value matches "
                             "the previous old value before outputting the old value.");

  params.addRequiredParam<VectorPostprocessorName>("vpp",
                                                   "The VectorPostprocessor vector to be coupled.");

  params.addRequiredParam<std::string>("vector_name",
                                       "The VectorPostprocessor vector to be coupled.");

  return params;
}

UseOldVectorPostprocessor::UseOldVectorPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _vec(getVectorPostprocessorValue("vpp", getParam<std::string>("vector_name"))),
    _vec_old(getVectorPostprocessorValueOld("vpp", getParam<std::string>("vector_name"))),
    _value(declareRestartableData<Real>("last_value", 0)),
    _old_value(declareRestartableData<Real>("last_old_value", 0)),
    _t_step(_fe_problem.timeStep()),
    _last_t_step(declareRestartableData<int>("last_t_step", 0))
{
}

void
UseOldVectorPostprocessor::execute()
{
  /**
   * We will only look at coupled values on processor 0 to avoid making assumptions about parallel
   * vectors.
   */
  if (processor_id() == 0)
  {
    // If we are repeating a timestep, for whatever reason, then we want to revert to the old value
    if (_t_step == _last_t_step)
      _value = _old_value;

    if (!_vec_old.empty())
    {
      // See if the old value matches the last current value we received.
      _old_value = _vec_old.back();

      if (_old_value != _value)
        mooseError("Probably with stateful VPP");
    }

    if (!_vec.empty())
      _value = _vec.back();

    _last_t_step = _t_step;
  }
}

PostprocessorValue
UseOldVectorPostprocessor::getValue() const
{
  return _old_value;
}
