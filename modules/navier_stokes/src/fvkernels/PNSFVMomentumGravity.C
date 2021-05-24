//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PNSFVMomentumGravity.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PNSFVMomentumGravity);
registerMooseObjectRenamed("NavierStokesApp",
                           PINSFVMomentumGravity,
                           "07/01/2021 00:00",
                           PNSFVMomentumGravity);

InputParameters
PNSFVMomentumGravity::validParams()
{
  InputParameters params = NSFVMomentumGravity::validParams();
  params.addClassDescription(
      "Computes a body force, $eps * \rho * g$ due to gravity on fluid in porous media.");
  params.addCoupledVar("porosity",
                       "Porosity auxiliary variable. If this is not supplied then we will attempt "
                       "to use a porosity material property");
  return params;
}

PNSFVMomentumGravity::PNSFVMomentumGravity(const InputParameters & params)
  : NSFVMomentumGravity(params),
    _eps(isCoupled("porosity") ? coupledValue("porosity")
                               : getMaterialProperty<Real>(NS::porosity).get())
{
}

ADReal
PNSFVMomentumGravity::computeQpResidual()
{
  return _eps[_qp] * NSFVMomentumGravity::computeQpResidual();
}
