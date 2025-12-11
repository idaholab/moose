//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MooseObject.h"

class MFEMPeriodicByVector : public MooseObject
{
public:
  static InputParameters validParams();
  MFEMPeriodicByVector(const InputParameters & parameters);
  mfem::Vector GetPeriodicBc(int dim);
  bool Use3D() const { return _3d; };

private:
  std::vector<Real> _translation_x;
  std::vector<Real> _translation_y;
  std::vector<Real> _translation_z;
  bool _3d = false;
};

#endif
