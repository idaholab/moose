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
#include "CoefficientManager.h"
#include "mfem.hpp"

#include <string>
/**
 * Abstract base class for all coordinate system scalar coefficient
 * built ins for MFEM formulations.Implemented as a MOOSE GeneralUserObject.
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

  // Build any coordinate dependent coefficient objects owned by this class.
  virtual void build() = 0;

  /// Declare all built-in coordinate coefficients into the coefficient manager
  virtual void declareCoefficients(Moose::MFEM::CoefficientManager & coeffs) = 0;

  /// Get a pointer to a built-in coefficient by name
  virtual const mfem::Coefficient * getBuiltinCoefficient(const std::string & name) const = 0;

  /// Return a fully-qualified coefficient name using this object's name as a prefix
  std::string coefficientName(const std::string & base) const { return name() + "_" + base; }

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
