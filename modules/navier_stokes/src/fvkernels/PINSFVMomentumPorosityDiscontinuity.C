//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPorosityDiscontinuity.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPorosityDiscontinuity);

InputParameters
PINSFVMomentumPorosityDiscontinuity::validParams()
{
  auto params = FVFluxKernel::validParams();
  params.addClassDescription("Porosity discontinuity term for the momentum equation.");
  params.addRequiredCoupledVar("porosity", "Porosity auxiliary variable");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<Real>("form_loss_coefficient", 1, "Form loss coefficient when viewing porosity changes as flow area reduction");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVMomentumPorosityDiscontinuity::PINSFVMomentumPorosityDiscontinuity(const InputParameters & params)
  : FVFluxKernel(params),
  _eps(coupledValue("porosity")),
  _eps_neighbor(coupledNeighborValue("porosity")),
  _index(getParam<MooseEnum>("momentum_component")),
  _K(getParam<Real>("form_loss_coefficient"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
PINSFVMomentumPorosityDiscontinuity::computeQpResidual()
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#else

  return _K * (_eps[_qp] - _eps_neighbor[_qp]) * _normal(_index);
#endif
}
