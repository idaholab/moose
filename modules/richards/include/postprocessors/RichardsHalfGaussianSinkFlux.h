/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSHALFGAUSSIANSINKFLUX_H
#define RICHARDSHALFGAUSSIANSINKFLUX_H

#include "SideIntegralVariablePostprocessor.h"
#include "RichardsVarNames.h"

class Function;

// Forward Declarations
class RichardsHalfGaussianSinkFlux;

template <>
InputParameters validParams<RichardsHalfGaussianSinkFlux>();

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
  Function & _m_func;

  /// porepressure (or porepressure vector for multiphase problems)
  const MaterialProperty<std::vector<Real>> & _pp;
};

#endif
