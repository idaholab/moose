/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MatVecRealGradAuxKernelAction.h"

template<>
InputParameters validParams<MatVecRealGradAuxKernelAction>()
{
  return validParams<Action>();
}

MatVecRealGradAuxKernelAction::MatVecRealGradAuxKernelAction(const InputParameters & params) :
    Action(params)
{
  mooseDeprecated("Use 'MaterialVectorAuxKernel' or 'MaterialVectorGradAuxKernel' action instead depending on data_type of MaterialProperty<std::vector<date_type> >");
}
