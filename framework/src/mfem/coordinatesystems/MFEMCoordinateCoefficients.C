//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCoordinateCoefficients.h"

InputParameters
MFEMCoordinateCoefficients::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<Real>(
      "inv_r_eps", 1e-12, "Floor epsilon used in inv_r = sqrt(1/(r^2 + eps^2)) to avoid axis singularity.");
  return params;
}

MFEMCoordinateCoefficients::MFEMCoordinateCoefficients(const InputParameters & parameters)
  : GeneralUserObject(parameters), _inv_r_eps(getParam<Real>("inv_r_eps"))
{
}

#endif
