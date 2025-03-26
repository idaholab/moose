//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorIC.h"
#include "Function.h"
#include "MooseFunctorArguments.h"
#include "UserObject.h"

registerMooseObject("MooseApp", FunctorIC);

InputParameters
FunctorIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params += NonADFunctorInterface::validParams();
  params.addRequiredParam<MooseFunctorName>("functor", "The initial condition functor.");

  params.addClassDescription("An initial condition that uses a normal function of x, y, z to "
                             "produce values (and optionally gradients) for a field variable.");
  params.addParam<Real>("scaling_factor", 1, "Scaling factor to apply on the function");

  return params;
}

FunctorIC::FunctorIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    NonADFunctorInterface(this),
    _functor(getFunctor<Real>("functor")),
    _scaling(getParam<Real>("scaling_factor"))
{
  // Check supported and unsupported functors
  const auto & functor_name = getParam<MooseFunctorName>("functor");
  // See https://github.com/idaholab/moose/issues/19396 for discussion of the functor restrictions.
  // Variables: need to be initialized before. we dont support this at the time
  if (_fe_problem.hasVariable(functor_name))
    paramError("functor",
               "Initializing a variable with another variable is not supported at this time");
  // Functions are supported
  else if (_fe_problem.hasFunction(functor_name) || MooseUtils::parsesToReal(functor_name))
  {
  }
  // Functor materials: fairly high risk since they could depend on variables
  else
    paramInfo("functor",
              "Functor materials or postprocessors should not depend on variables if used in a "
              "FunctorIC");
}

Real
FunctorIC::value(const Point & p)
{
  // TODO: This is because ICs are created before PPs. This would be nicer in an initialSetup of the
  // IC which do not currently exist
  mooseDoOnce( // PPs: need to execute before the ICs are
      const auto & functor_name = getParam<MooseFunctorName>("functor");
      if (_fe_problem.hasPostprocessorValueByName(functor_name)) {
        if (!(_fe_problem.getUserObjectBase(functor_name).isParamValid("force_preic") &&
              _fe_problem.getUserObjectBase(functor_name).getParam<bool>("force_preic")))
          paramError("functor",
                     "Functor is a postprocessor and does not have 'force_preic' set to true");
      });

  // Use nodes for nodal-defined variables, elements for the others
  if (_var.isNodalDefined())
  {
    Moose::NodeArg node_arg = {_current_node,
                               blockRestricted() ? &blockIDs()
                                                 : &Moose::NodeArg::undefined_subdomain_connection};
    return _scaling * _functor(node_arg, Moose::currentState());
  }
  else
  {
    Moose::ElemPointArg elem_point = {_current_elem, p, false};
    return _scaling * _functor(elem_point, Moose::currentState());
  }
}

RealGradient
FunctorIC::gradient(const Point & p)
{
  // Use nodes for nodal-defined variables, elements for the others
  if (_var.isNodalDefined())
  {
    Moose::NodeArg node_arg = {_current_node,
                               blockRestricted() ? &blockIDs()
                                                 : &Moose::NodeArg::undefined_subdomain_connection};
    return _scaling * _functor.gradient(node_arg, Moose::currentState());
  }
  else
  {
    Moose::ElemPointArg elem_point = {_current_elem, p, false};
    return _scaling * _functor.gradient(elem_point, Moose::currentState());
  }
}
