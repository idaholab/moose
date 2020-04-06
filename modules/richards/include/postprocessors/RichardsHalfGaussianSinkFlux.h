//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralVariablePostprocessor.h"
#include "RichardsVarNames.h"

class Function;

// Forward Declarations

/**
 * Postprocessor that records the mass flux from porespace to
 * a half-gaussian sink.  (Positive if fluid is being removed from porespace.)
 * flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and flux out = max otherwise
 * If a function, _m_func, is used then the flux is multiplied by _m_func.
 * The result is the flux integrated over the specified sideset.
 */
class RichardsHalfGaussianSinkFlux : public SideIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  RichardsHalfGaussianSinkFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  FEProblemBase & _feproblem;

  /// flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and flux out = max otherwise
  Real _maximum;

  /// flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and flux out = max otherwise
  Real _sd;

  /// flux out = max*exp((-0.5*(p - centre)/sd)^2) for p<centre, and flux out = max otherwise
  Real _centre;

  /**
   * holds info regarding the names of the Richards variables
   * and methods for extracting values of these variables
   */
  const RichardsVarNames & _richards_name_UO;

  /**
   * the index of this variable in the list of Richards variables
   * held by _richards_name_UO.  Eg
   * if richards_vars = 'pwater pgas poil' in the _richards_name_UO
   * and this kernel has variable = pgas, then _pvar = 1
   * This is used to index correctly into _viscosity, _seff, etc
   */
  unsigned int _pvar;

  /// the multiplier function
  const Function & _m_func;

  /// porepressure (or porepressure vector for multiphase problems)
  const MaterialProperty<std::vector<Real>> & _pp;
};
