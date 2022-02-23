//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReynoldsNumberFunctorAux.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", ReynoldsNumberFunctorAux);

InputParameters
ReynoldsNumberFunctorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes rho*u*L/mu.");
  params.addRequiredParam<MooseFunctorName>(NS::speed, "The fluid speed");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "The fluid dynamic viscosity");
  params.addParam<bool>(
      "use_qp_arg",
      false,
      "Whether to use the quadrature point based functor agument to evaluate the functors. If "
      "false, then the argument is element-based, which is more appropriate for finite volume. A "
      "value of true is more appropriate for finite element computations.");
  return params;
}

ReynoldsNumberFunctorAux::ReynoldsNumberFunctorAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _speed(getFunctor<ADReal>(NS::speed)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _use_qp_arg(getParam<bool>("use_qp_arg"))
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

Real
ReynoldsNumberFunctorAux::computeValue()
{
  using MetaPhysicL::raw_value;
  if (_use_qp_arg)
  {
    const auto qp_arg = std::make_tuple(_current_elem, _qp, _qrule);
    return _current_elem->hmax() * raw_value(_rho(qp_arg)) * raw_value(_speed(qp_arg)) /
           raw_value(_mu(qp_arg));
  }
  else
  {
    const auto elem_arg = makeElemArg(_current_elem);
    return _current_elem->hmax() * raw_value(_rho(elem_arg)) * raw_value(_speed(elem_arg)) /
           raw_value(_mu(elem_arg));
  }
}
