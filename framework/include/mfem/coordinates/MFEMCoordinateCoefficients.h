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

#include "GeneralUserObject.h"
#include "MooseTypes.h"
#include "mfem.hpp"

#include <string>

class MFEMCoordinateCoefficients : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMCoordinateCoefficients(const InputParameters & parameters);
  virtual ~MFEMCoordinateCoefficients() = default;

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  virtual void build() = 0;

  virtual const mfem::Coefficient * getBuiltinCoefficient(const std::string & name) const = 0;

  virtual const mfem::Coefficient * getRadialCoefficient() const = 0;
  virtual const mfem::Coefficient * getInverseRadialCoefficient() const = 0;
  virtual const mfem::Coefficient * getTwoPiRCoefficient() const = 0;
  virtual const mfem::Coefficient * getMeasureWeightCoefficient() const = 0;

protected:
  const Real _inv_r_eps;
};

#endif
