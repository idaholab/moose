//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTV2SourceSink.h"
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", INSFVTV2SourceSink);

InputParameters
INSFVTV2SourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Elemental kernel to compute the production and destruction "
                             " terms of the velocity fluctations normal to the wall (v2).");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation.");
  params.addRequiredParam<MooseFunctorName>(NS::TF, "Coupled turbulent relaxation function.");
  params.addParam<Real>("n", 6, "Model parameter.");

  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVTV2SourceSink::INSFVTV2SourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _f(getFunctor<ADReal>(NS::TF)),
    _n(getParam<Real>("n"))
{
}

ADReal
INSFVTV2SourceSink::computeQpResidual()
{
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  const auto TKE = _k(elem_arg, state);
  const auto TKED = _epsilon(elem_arg, state);
  const auto V2 = _var(elem_arg, state);

  const auto production = TKE * _f(elem_arg, state);
  const auto destruction = _n * V2 * TKED / (TKE + 1e-15);

  return destruction - production;
}
