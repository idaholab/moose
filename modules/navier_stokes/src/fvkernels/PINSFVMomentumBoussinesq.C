//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumBoussinesq.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumBoussinesq);

InputParameters
PINSFVMomentumBoussinesq::validParams()
{
  InputParameters params = INSFVMomentumBoussinesq::validParams();
  params.addClassDescription(
      "Computes a body force for natural convection buoyancy in porous media: eps alpha (T-T_0)");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity auxiliary variable");

  return params;
}

PINSFVMomentumBoussinesq::PINSFVMomentumBoussinesq(const InputParameters & params)
  : INSFVMomentumBoussinesq(params), _eps(getFunctor<ADReal>(NS::porosity))
{
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError("PINSFVMomentumBoussinesq may only be used with a superficial velocity "
               "variable, of variable type PINSFVSuperficialVelocityVariable.");
}

void
PINSFVMomentumBoussinesq::gatherRCData(const Elem & elem)
{
  const auto elem_arg = makeElemArg(&elem);
  _rc_uo.addToB(&elem,
                _index,
                _eps(elem_arg) * _alpha(elem_arg) * _gravity(_index) * _rho *
                    (_temperature(elem_arg) - _ref_temperature));
}
