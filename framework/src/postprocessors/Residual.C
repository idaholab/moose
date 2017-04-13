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

#include "Residual.h"

#include "FEProblem.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<Residual>()
{
  MooseEnum residual_types("FINAL INITIAL_BEFORE_PRESET INITIAL_AFTER_PRESET", "FINAL");

  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<MooseEnum>("residual_type",
                             residual_types,
                             "Type of residual to be reported.  Choices are: " +
                                 residual_types.getRawNames());
  return params;
}

Residual::Residual(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _residual_type(getParam<MooseEnum>("residual_type"))
{
}

Real
Residual::getValue()
{
  Real residual = 0.0;
  if (_residual_type == "FINAL")
    residual = _subproblem.finalNonlinearResidual();
  else
  {
    FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
    if (!fe_problem)
      mooseError("Dynamic cast to FEProblemBase failed in Residual Postprocessor");
    if (_residual_type == "INITIAL_BEFORE_PRESET")
      residual = fe_problem->getNonlinearSystemBase()._initial_residual_before_preset_bcs;
    else if (_residual_type == "INITIAL_AFTER_PRESET")
      residual = fe_problem->getNonlinearSystemBase()._initial_residual_after_preset_bcs;
    else
      mooseError("Invalid residual_type option in Residual Postprocessor: ", _residual_type);
  }
  return residual;
}
