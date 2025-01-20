//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IPHDGPrescribedFluxBC.h"
#include "IPHDGAssemblyHelper.h"

InputParameters
IPHDGPrescribedFluxBC::validParams()
{
  auto params = IntegratedBC::validParams();
  params.addParam<MooseFunctorName>(
      "prescribed_normal_flux", 0, "The prescribed value of the flux dotted with the normal");
  return params;
}

IPHDGPrescribedFluxBC::IPHDGPrescribedFluxBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    ADFunctorInterface(this),
    _normal_flux(getFunctor<Real>("prescribed_normal_flux")),
    _my_elem(nullptr),
    _my_side(libMesh::invalid_uint)
{
}

void
IPHDGPrescribedFluxBC::compute()
{
  auto & iphdg_helper = iphdgHelper();
  iphdg_helper.resizeResiduals();

  // For notation, please read "A superconvergent LDG-hybridizable Galerkin method for second-order
  // elliptic problems" by Cockburn

  // u, lm_u
  iphdg_helper.scalarFace();
  iphdg_helper.lmFace();
  iphdg_helper.lmPrescribedFlux(_normal_flux);
}

void
IPHDGPrescribedFluxBC::computeResidual()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResiduals(_assembly, residual_packet);
}

void
IPHDGPrescribedFluxBC::computeJacobian()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addJacobian(_assembly, residual_packet);
}

void
IPHDGPrescribedFluxBC::computeResidualAndJacobian()
{
  compute();
  for (const auto & residual_packet : iphdgHelper().taggingData())
    addResidualsAndJacobian(_assembly, residual_packet);
}

void
IPHDGPrescribedFluxBC::jacobianSetup()
{
  _my_elem = nullptr;
  _my_side = libMesh::invalid_uint;
}

void
IPHDGPrescribedFluxBC::computeOffDiagJacobian(const unsigned int)
{
  if ((_my_elem != _current_elem) || (_my_side != _current_side))
  {
    computeJacobian();
    _my_elem = _current_elem;
    _my_side = _current_side;
  }
}
