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
  params.addParam<std::vector<Real>>("translation_x",
                                     "Vector specifying translation in x direction.");
  params.addParam<std::vector<Real>>("translation_y",
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
  }
}

std::vector<mfem::Vector>
MFEMPeriodicByVector::GetPeriodicBCs()
{
  std::vector<mfem::Vector> output;

  // check if each translation vector is populated, then convert
  // to mfem vector and insert into the std vector which we return
  if (_translation_x.size())
  {
    mfem::Vector translation_x(_translation_x.size());
    // copy entries
    translation_x = _translation_x.data();
    output.push_back(translation_x);
  }

  if (_translation_y.size())
  {
    mfem::Vector translation_y(_translation_y.size());
    // copy entries
    translation_y = _translation_y.data();
    output.push_back(translation_y);
  }

  if (_translation_z.size())
  {
    mfem::Vector translation_z(_translation_z.size());
    // copy entries
    translation_z = _translation_z.data();
    output.push_back(translation_z);
  }

  return output;
}

#endif
