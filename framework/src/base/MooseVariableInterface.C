//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableInterface.h"

#include "Assembly.h"
#include "MooseError.h" // mooseDeprecated
#include "MooseTypes.h"
#include "MooseVariableField.h"
#include "Problem.h"
#include "SubProblem.h"

template <typename T>
MooseVariableInterface<T>::MooseVariableInterface(const MooseObject * moose_object,
                                                  bool nodal,
                                                  std::string var_param_name)
  : _nodal(nodal)
{
  const InputParameters & parameters = moose_object->parameters();

  SubProblem & problem = *parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");

  // Try the scalar version first
  std::string variable_name = parameters.getMooseType(var_param_name);
  if (variable_name == "")
    // When using vector variables, we are only going to use the first one in the list at the
    // interface level...
    variable_name = parameters.getVecMooseType(var_param_name)[0];

  _variable = &dynamic_cast<MooseVariableField<T> &>(problem.getVariable(tid, variable_name));

  _mvi_assembly = &problem.assembly(tid);
}

template <typename T>
MooseVariableInterface<T>::~MooseVariableInterface()
{
}

template <typename T>
MooseVariableField<T> *
MooseVariableInterface<T>::mooseVariable()
{
  return _variable;
}

template <typename T>
const typename OutputTools<T>::VariableValue &
MooseVariableInterface<T>::value()
{
  if (_nodal)
    return _variable->nodalValue();
  else
    return _variable->sln();
}

template <>
const VectorVariableValue &
MooseVariableInterface<RealVectorValue>::value()
{
  if (_nodal)
    mooseError("Dofs are scalars while vector variables have vector values. Mismatch");
  else
    return _variable->sln();
}

template <typename T>
const typename OutputTools<T>::VariableValue &
MooseVariableInterface<T>::valueOld()
{
  if (_nodal)
    return _variable->nodalValueOld();
  else
    return _variable->slnOld();
}

template <>
const VectorVariableValue &
MooseVariableInterface<RealVectorValue>::valueOld()
{
  if (_nodal)
    mooseError("Dofs are scalars while vector variables have vector values. Mismatch");
  else
    return _variable->slnOld();
}

template <typename T>
const typename OutputTools<T>::VariableValue &
MooseVariableInterface<T>::valueOlder()
{
  if (_nodal)
    return _variable->nodalValueOlder();
  else
    return _variable->slnOlder();
}

template <>
const VectorVariableValue &
MooseVariableInterface<RealVectorValue>::valueOlder()
{
  if (_nodal)
    mooseError("Dofs are scalars while vector variables have vector values. Mismatch");
  else
    return _variable->slnOlder();
}

template <typename T>
const typename OutputTools<T>::VariableValue &
MooseVariableInterface<T>::dot()
{
  if (_nodal)
    return _variable->nodalValueDot();
  else
    return _variable->uDot();
}

template <>
const VectorVariableValue &
MooseVariableInterface<RealVectorValue>::dot()
{
  if (_nodal)
    mooseError("Dofs are scalars while vector variables have vector values. Mismatch");
  else
    return _variable->uDot();
}

template <typename T>
const typename OutputTools<T>::VariableValue &
MooseVariableInterface<T>::dotDu()
{
  if (_nodal)
    return _variable->nodalValueDuDotDu();
  else
    return _variable->duDotDu();
}

template <>
const VectorVariableValue &
MooseVariableInterface<RealVectorValue>::dotDu()
{
  if (_nodal)
    mooseError("Dofs are scalars while vector variables have vector values. Mismatch");
  else
    return _variable->duDotDu();
}

template <typename T>
const typename OutputTools<T>::VariableGradient &
MooseVariableInterface<T>::gradient()
{
  if (_nodal)
    mooseError("gradients are not defined at nodes");

  return _variable->gradSln();
}

template <typename T>
const typename OutputTools<T>::VariableGradient &
MooseVariableInterface<T>::gradientOld()
{
  if (_nodal)
    mooseError("gradients are not defined at nodes");

  return _variable->gradSlnOld();
}

template <typename T>
const typename OutputTools<T>::VariableGradient &
MooseVariableInterface<T>::gradientOlder()
{
  if (_nodal)
    mooseError("gradients are not defined at nodes");

  return _variable->gradSlnOlder();
}

template <typename T>
const typename OutputTools<T>::VariableSecond &
MooseVariableInterface<T>::second()
{
  if (_nodal)
    mooseError("second derivatives are not defined at nodes");

  return _variable->secondSln();
}

template <typename T>
const typename OutputTools<T>::VariableSecond &
MooseVariableInterface<T>::secondOld()
{
  if (_nodal)
    mooseError("second derivatives are not defined at nodes");

  return _variable->secondSlnOld();
}

template <typename T>
const typename OutputTools<T>::VariableSecond &
MooseVariableInterface<T>::secondOlder()
{
  if (_nodal)
    mooseError("second derivatives are not defined at nodes");

  return _variable->secondSlnOlder();
}

template <typename T>
const typename OutputTools<T>::VariableTestSecond &
MooseVariableInterface<T>::secondTest()
{
  if (_nodal)
    mooseError("second derivatives are not defined at nodes");

  return _variable->secondPhi();
}

template <typename T>
const typename OutputTools<T>::VariableTestSecond &
MooseVariableInterface<T>::secondTestFace()
{
  if (_nodal)
    mooseError("second derivatives are not defined at nodes");

  return _variable->secondPhiFace();
}

template <typename T>
const typename OutputTools<T>::VariablePhiSecond &
MooseVariableInterface<T>::secondPhi()
{
  if (_nodal)
    mooseError("second derivatives are not defined at nodes");

  return _mvi_assembly->secondPhi(*_variable);
}

template <typename T>
const typename OutputTools<T>::VariablePhiSecond &
MooseVariableInterface<T>::secondPhiFace()
{
  if (_nodal)
    mooseError("second derivatives are not defined at nodes");

  return _mvi_assembly->secondPhiFace(*_variable);
}

template class MooseVariableInterface<Real>;
template class MooseVariableInterface<RealVectorValue>;
