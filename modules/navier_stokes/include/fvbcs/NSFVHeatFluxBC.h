//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"
#include "NSEnums.h"

/**
 * This boundary condition sets a constant heat flux with a splitting between the
 * fluid and solid phases according to one of
 *
 * - local value of porosity
 * - local value of thermal conductivity
 * - local value of effective thermal conductivity
 * - domain-averaged value of porosity
 * - domain-averaged value of thermal conductivity
 * - domain-averaged value of effective thermal conductivity
 *
 * Local values are obtained on the boundary, while domain-averaged, or _global_, values
 * are computed as averages over the domain. This boundary condition does not specify the
 * interpretation of what constitutes the domain, so you may choose for the global values to
 * represent averages over the entire geometry, or possibly a finite-width region near the wall.
 *
 * For instance, if the heat flux is split according to the local value of thermal
 * conductivity, the heat flux entering the fluid phase is
 * \f$\frac{k_f}{k_f+k_s}\dot{q}\f$, where \f$k_f\f$ is the fluid thermal conductivity,
 * \f$k_s\f$ is the solid thermal conductivity, and \f$\dot{q}\f$ is the total value of
 * the heat flux.
 *
 * Neither selection of a local or global splitting is perfect.
 */
class NSFVHeatFluxBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  NSFVHeatFluxBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Value of the heat flux
  const Real & _value;

  /// Which phase this boundary condition is applied to, i.e. 'fluid' or 'solid'
  const NS::phase::PhaseEnum _phase;

  /**
   * What parameters are used to split the heat flux, i.e. 'thermal conductivity',
   * 'effective thermal conductivity', or 'porosity'.
   * To protect against cases where at the first time step
   * the thermal conductivity or effective thermal conductivity might not have yet
   * been initialized, or cases where the coupled postprocessors have not yet been
   * evaluated, we use an equal flux splitting.
   */
  const NS::splitting::SplittingEnum _split_type;

  /**
   * Where the values used in computing the splitting are pulled from, i.e. 'local'
   * or 'global'.
   */
  const NS::settings::LocalityEnum _locality;

  /// Domain-average porosity
  const PostprocessorValue * _average_eps;

  /// Domain-average fluid thermal conductivity
  const PostprocessorValue * _average_k_f;

  /// Domain-average solid thermal conductivity
  const PostprocessorValue * _average_k_s;

  /// Domain-average solid effective thermal conductivity
  const PostprocessorValue * _average_kappa_s;

  /// Domain-average fluid effective thermal conductivity
  const PostprocessorValue * _average_kappa;

  /// Porosity
  const VariableValue & _eps;

  /// Fluid thermal conductivity
  const ADMaterialProperty<Real> * _k_f;

  /// Solid thermal conductivity
  const ADMaterialProperty<Real> * _k_s;

  /// Fluid effective thermal conductivity
  const ADMaterialProperty<RealVectorValue> * _kappa;

  /// Solid effective thermal conductivity
  const ADMaterialProperty<Real> * _kappa_s;
};
