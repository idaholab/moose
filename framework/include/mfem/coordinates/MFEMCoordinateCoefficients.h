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
/**
 * Abstract base class for all coordinate system coefficient providers.
 * Implemented as a MOOSE GeneralUserObject.
 */
class MFEMCoordinateCoefficients : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMCoordinateCoefficients(const InputParameters & parameters);
  virtual ~MFEMCoordinateCoefficients() = default;
  ///GeneralUserObject interface
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
  // Virtual methods to be implemented by the derived classes
  virtual void build() = 0;
  /// Get a pointer to a built-in coefficient by name (makes built-in coefficients accessible to kernels and BCs through the '[Coordinates]' block)
  virtual const mfem::Coefficient * getBuiltinCoefficient(const std::string & name) const = 0;
  ///Pointer getters for coefficients in coordinate systems with radial dependence
  virtual const mfem::Coefficient * getRadialCoefficient() const = 0;
  virtual const mfem::Coefficient * getInverseRadialCoefficient() const = 0;
  virtual const mfem::Coefficient * getTwoPiRCoefficient() const = 0;
  virtual const mfem::Coefficient * getMeasureWeightCoefficient() const = 0;

protected:
  /// Optional parameters for coordinate coefficients with radial dependence
  const Real _inv_r_eps;
};

#endif
