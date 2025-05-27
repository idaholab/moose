//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  auto params = IPHDGBC::validParams();
  params.addRequiredParam<MooseFunctorName>(
      "prescribed_normal_flux", "The prescribed value of the flux dotted with the normal");
  return params;
}

IPHDGPrescribedFluxBC::IPHDGPrescribedFluxBC(const InputParameters & parameters)
  : IPHDGBC(parameters), _normal_flux(getFunctor<Real>("prescribed_normal_flux"))
{
}

void
IPHDGPrescribedFluxBC::compute()
{
  auto & iphdg_helper = iphdgHelper();
  iphdg_helper.resizeResiduals();

  // u, lm_u
  iphdg_helper.scalarFace();
  iphdg_helper.lmFace();
  iphdg_helper.lmPrescribedFlux(_normal_flux);
}
