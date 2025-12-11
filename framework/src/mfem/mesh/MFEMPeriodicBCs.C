//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPeriodicBCs.h"

registerMooseObject("MooseApp", MFEMPeriodicByVector);

InputParameters
MFEMPeriodicByVector::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("MFEMPeriodicByVector");
  params.addRequiredParam<std::vector<Real>>("translation_x",
                                             "Vector specifying translation in x direction.");
  params.addRequiredParam<std::vector<Real>>("translation_y",
                                             "Vector specifying translation in y direction.");
  params.addParam<std::vector<Real>>("translation_z",
                                     "Vector specifying translation in z direction.");
  return params;
}

MFEMPeriodicByVector::MFEMPeriodicByVector(const InputParameters & parameters)
  : MooseObject(parameters)
{
  _translation_x = getParam<std::vector<Real>>("translation_x");
  _translation_y = getParam<std::vector<Real>>("translation_y");
  if (isParamSetByUser("translation_z"))
  {
    _translation_z = getParam<std::vector<Real>>("translation_z");
    _3d = true;
  }
}

mfem::Vector
MFEMPeriodicByVector::GetPeriodicBc(int dim)
{
  // set the size from the x vector
  mfem::Vector output(_translation_x.size());

  if (dim == 0)
  {
    output = _translation_x.data();
  }

  else if (dim == 1)
  {
    output = _translation_y.data();
  }

  else if (dim == 2)
  {
    output = _translation_z.data();
  }

  else
  {
    mooseError("Dimension should be between 0 and 2 inclusive");
  }
  return output;
}

#endif
