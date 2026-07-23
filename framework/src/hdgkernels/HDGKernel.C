//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGKernel.h"
#include "HDGAssemblyHelper.h"
#include "TimeIntegrator.h"

InputParameters
HDGKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.registerBase("HDGKernel");
  return params;
}

HDGKernel::HDGKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _qrule_face(_assembly.qRuleFace()),
    _q_point_face(_assembly.qPointsFace()),
    _JxW_face(_assembly.JxWFace()),
    _normals(_assembly.normals()),
    _current_side_elem(_assembly.sideElem()),
    _hdg_cached_elem(nullptr)
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
HDGKernel::compute()
{
  auto * const helper = hdgHelper();
  mooseAssert(helper, "Helper-driven HDG assembly requires a non-null helper");
  helper->resizeResiduals();
  helper->scalarVolume();
}

void
HDGKernel::computeOnSide()
{
  auto * const helper = hdgHelper();
  mooseAssert(helper, "Helper-driven HDG assembly requires a non-null helper");
  helper->resizeResiduals();
  helper->scalarFace();
  helper->lmFace();
}

void
HDGKernel::computeResidual()
{
  compute();
  for (const auto & residual_packet : hdgHelper()->taggingData())
    addResiduals(_assembly, residual_packet);
}

void
HDGKernel::computeJacobian()
{
  compute();
  for (const auto & residual_packet : hdgHelper()->taggingData())
    addJacobian(_assembly, residual_packet);
}

void
HDGKernel::computeResidualAndJacobian()
{
  if (!hdgHelper())
    return Kernel::computeResidualAndJacobian();

  compute();
  for (const auto & residual_packet : hdgHelper()->taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
HDGKernel::computeResidualOnSide()
{
  computeOnSide();
  for (const auto & residual_packet : hdgHelper()->taggingData())
    addResiduals(_assembly, residual_packet);
}

void
HDGKernel::computeJacobianOnSide()
{
  computeOnSide();
  for (const auto & residual_packet : hdgHelper()->taggingData())
    addJacobian(_assembly, residual_packet);
}

void
HDGKernel::computeResidualAndJacobianOnSide()
{
  if (!hdgHelper())
  {
    computeResidualOnSide();
    computeJacobianOnSide();
    return;
  }

  computeOnSide();
  for (const auto & residual_packet : hdgHelper()->taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
HDGKernel::jacobianSetup()
{
  _hdg_cached_elem = nullptr;
}

void
HDGKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_hdg_cached_elem != _current_elem)
  {
    computeJacobian();
    _hdg_cached_elem = _current_elem;
  }
}

std::set<std::string>
HDGKernel::additionalROVariables()
{
  if (auto * const helper = hdgHelper())
    return helper->additionalROVariables();
  return Kernel::additionalROVariables();
}

const std::unordered_set<unsigned int> &
HDGKernel::getMatPropDependencies() const
{
  if (const auto * const helper = hdgHelper())
    return helper->getMatPropDependencies();
  return Kernel::getMatPropDependencies();
}

bool
HDGKernel::getMaterialPropertyCalled() const
{
  if (const auto * const helper = hdgHelper())
    return helper->getMaterialPropertyCalled();
  return Kernel::getMaterialPropertyCalled();
}
