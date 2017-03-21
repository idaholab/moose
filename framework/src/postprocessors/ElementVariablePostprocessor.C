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

#include "ElementVariablePostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ElementVariablePostprocessor>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this postprocessor operates on");
  return params;
}

ElementVariablePostprocessor::ElementVariablePostprocessor(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    MooseVariableInterface(this, false),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable")),
    _u_dot(coupledDot("variable")),
    _qp(0)
{
  addMooseVariableDependency(mooseVariable());
}

void
ElementVariablePostprocessor::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    computeQpValue();
}
