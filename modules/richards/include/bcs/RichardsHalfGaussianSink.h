//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "RichardsVarNames.h"

// Forward Declarations

/**
 * Applies a fluid sink to the boundary.
 * The sink has strength
 * _maximum*exp(-(0.5*(p - c)/_sd)^2)*_m_func for p<c
 * _maximum*_m_func for p>=c
 * This is typically used for modelling evapotranspiration
 * from the top of a groundwater model
 */
class RichardsHalfGaussianSink : public IntegratedBC
{
public:
  static InputParameters validParams();

  RichardsHalfGaussianSink(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// maximum of the Gaussian sink
  Real _maximum;

  /// standard deviation of the Gaussian sink
  Real _sd;

  /// centre of the Gaussian sink
  Real _centre;

  /// multiplying function: all fluxes will be multiplied by this
  const Function & _m_func;

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

  /// porepressure (or porepressure vector for multiphase problems)
  const MaterialProperty<std::vector<Real>> & _pp;

  /// d(porepressure_i)/dvariable_j
  const MaterialProperty<std::vector<std::vector<Real>>> & _dpp_dv;
};
