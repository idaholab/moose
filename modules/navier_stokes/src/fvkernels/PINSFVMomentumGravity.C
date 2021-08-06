//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumGravity.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumGravity);

InputParameters
PINSFVMomentumGravity::validParams()
{
  InputParameters params = INSFVMomentumGravity::validParams();
  params.addClassDescription("Computes a body force, $eps * \rho * g$ due to gravity on fluid in "
                             "porous media in Rhie-Chow (incompressible) contexts.");
  params.addParam<MooseFunctorName>(NS::porosity, NS::porosity, "Porosity functor");
  return params;
}

PINSFVMomentumGravity::PINSFVMomentumGravity(const InputParameters & params)
  : INSFVMomentumGravity(params), _eps(getFunctor<ADReal>(NS::porosity))
{
}

void
PINSFVMomentumGravity::gatherRCData(const Elem & elem)
{
  const auto elem_arg = makeElemArg(&elem);
  _rc_uo.addToB(&elem, _index, _eps(elem_arg) * -_rho(elem_arg) * _gravity(_index));
}
