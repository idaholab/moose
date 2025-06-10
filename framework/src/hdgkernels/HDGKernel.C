//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGKernel.h"
#include "TimeIntegrator.h"

InputParameters
HDGKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.registerBase("HDGKernel");
  return params;
}

HDGKernel::HDGKernel(const InputParameters & parameters) : Kernel(parameters)
{
  if (const auto * const ti = _sys.queryTimeIntegrator(_var.number()); ti && ti->isExplicit())
    mooseError("HDGKernels do not currently work with explicit time integration. This is because "
               "the facet Lagrange multiplier variable does not have a time derivative term.");

  const auto coord_system = _mesh.getUniqueCoordSystem();
  if (coord_system != Moose::COORD_XYZ)
    mooseError("HDGKernels have not yet been coded to include coordinate system information in its "
               "residuals/Jacobians");
}

void
HDGKernel::computeResidualAndJacobianOnSide()
{
  mooseError("not implemented");
}
