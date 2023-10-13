//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVRhieChowInterpolatorSegregated.h"
#include "NS.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", PINSFVRhieChowInterpolatorSegregated);

InputParameters
PINSFVRhieChowInterpolatorSegregated::validParams()
{
  auto params = INSFVRhieChowInterpolatorSegregated::validParams();

  params.addClassDescription("Computes H/A and 1/A together with face velocities for segregated "
                             "porous medium momentum-pressure equations.");

  params.addParam<MooseFunctorName>(NS::porosity, "The porosity functor.");

  return params;
}

PINSFVRhieChowInterpolatorSegregated::PINSFVRhieChowInterpolatorSegregated(
    const InputParameters & params)
  : INSFVRhieChowInterpolatorSegregated(params),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _epss(libMesh::n_threads(), nullptr)
{
  const auto porosity_name = deduceFunctorName(NS::porosity);
  for (const auto tid : make_range(libMesh::n_threads()))
    _epss[tid] = &UserObject::_subproblem.getFunctor<ADReal>(porosity_name, tid, name(), true);
}
