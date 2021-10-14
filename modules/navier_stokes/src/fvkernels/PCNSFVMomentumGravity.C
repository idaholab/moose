//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVMomentumGravity.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PCNSFVMomentumGravity);

InputParameters
PCNSFVMomentumGravity::validParams()
{
  InputParameters params = CNSFVMomentumGravity::validParams();
  params.addClassDescription(
      "Computes a body force, $eps * \rho * g$ due to gravity on fluid in porous media.");
  params.addCoupledVar("porosity",
                       "Porosity auxiliary variable. If this is not supplied then we will attempt "
                       "to use a porosity material property");
  return params;
}

PCNSFVMomentumGravity::PCNSFVMomentumGravity(const InputParameters & params)
  : CNSFVMomentumGravity(params),
    _eps(isCoupled("porosity") ? coupledValue("porosity")
                               : getMaterialProperty<Real>(NS::porosity).get())
{
}

ADReal
PCNSFVMomentumGravity::computeQpResidual()
{
  return _eps[_qp] * CNSFVMomentumGravity::computeQpResidual();
}
