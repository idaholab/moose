//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumPressure.h"

registerMooseObject("NavierStokesApp", INSFVMomentumPressure);

InputParameters
INSFVMomentumPressure::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Introduces the coupled pressure term into the Navier-Stokes momentum equation.");
  params.addRequiredCoupledVar("p", "The pressure");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

INSFVMomentumPressure::INSFVMomentumPressure(const InputParameters & params)
  : FVElementalKernel(params),
    _p_var(dynamic_cast<const MooseVariableFVReal *>(getFieldVar("p", 0))),
    _index(getParam<MooseEnum>("momentum_component"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!_p_var)
    paramError("p", "p must be a finite volume variable");
}

ADReal
INSFVMomentumPressure::computeQpResidual()
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#else
  return _p_var->adGradSln(_current_elem)(_index);
#endif
}
