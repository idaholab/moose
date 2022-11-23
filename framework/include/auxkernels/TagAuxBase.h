//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "MooseVariableBase.h"
#include "MooseVariableFieldBase.h"

/**
 * The value of a tagged vector for a given node and a given variable is coupled to
 * the current AuxVariable. TagVectorAux returns the coupled nodal value.
 */
template <class T>
class TagAuxBase : public T
{
public:
  static InputParameters validParams();

  TagAuxBase(const InputParameters & parameters);

protected:
  const bool _scaled;
  /**
   * check the aux variable acting on this auxkernl has the consistent properties with the input
   * coupled variable
   * @param input_var Input coupled variable
   * @param aux_var Aux variable
   */
  void checkCoupledVariable(const MooseVariableBase * input_var,
                            const MooseVariableFieldBase * aux_var);

  using T::_var;
  using T::paramError;
};

template <class T>
InputParameters
TagAuxBase<T>::validParams()
{
  InputParameters params = T::validParams();

  params.addRequiredCoupledVar("v",
                               "The coupled variable whose components are coupled to AuxVariable");
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_TIMESTEP_END};
  params.addParam<bool>("scaled",
                        true,
                        "Return value depending on the variable scaling/autoscaling. Set this to "
                        "false to obtain unscaled physical reaction forces.");
  return params;
}

template <class T>
TagAuxBase<T>::TagAuxBase(const InputParameters & parameters)
  : T(parameters), _scaled(this->template getParam<bool>("scaled"))
{
  auto & execute_on = this->template getParam<ExecFlagEnum>("execute_on");
  if (execute_on.size() != 1 || !execute_on.contains(EXEC_TIMESTEP_END))
    paramError("execute_on", "must be set to EXEC_TIMESTEP_END");
}

template <class T>
void
TagAuxBase<T>::checkCoupledVariable(const MooseVariableBase * input_var,
                                    const MooseVariableFieldBase * aux_var)
{
  if (input_var->feType() != aux_var->feType())
    paramError("variable",
               "The AuxVariable this AuxKernel is acting on has to have the same order and family "
               "as the variable 'v'");
  if (input_var->count() != aux_var->count())
    paramError("variable",
               "The AuxVariable this AuxKernel is acting on has to have the same number of "
               "components as the variable 'v'");
}
