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
#include "GapConductance.h"

class PenetrationInfo;

/**
 * Generic gap heat transfer model, with h_gap =  h_conduction + h_contact + h_radiation
 */
class GapHeatTransfer : public IntegratedBC
{
public:
  static InputParameters validParams();

  GapHeatTransfer(const InputParameters & parameters);

  virtual void initialSetup() override;
  void computeJacobian() override;

  void computeOffDiagJacobian(unsigned int jvar) override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /**
   * compute the Jacobian contributions from the secondary side degrees of freedom
   */
  Real computeSecondaryQpJacobian();

  /**
   * compute the displacement Jacobian contributions from the secondary side degrees of freedom
   */
  Real computeSecondaryQpOffDiagJacobian(unsigned int jvar);

  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;

  virtual Real gapLength() const;
  virtual Real dgapLength(Real normalComponent) const;
  virtual Real computeSecondaryFluxContribution(Real grad_t);
  virtual void computeGapValues();

  GapConductance::GAP_GEOMETRY & _gap_geometry_type;

  const bool _quadrature;

  NumericVector<Number> * _secondary_flux;

  const MaterialProperty<Real> & _gap_conductance;
  const MaterialProperty<Real> & _gap_conductance_dT;

  const Real _min_gap;
  const unsigned int _min_gap_order;
  const Real _max_gap;

  Real _gap_temp;
  Real _gap_distance;
  Real _radius;
  Real _r1;
  Real _r2;

  /**
   * This is a factor that is used to gradually taper down the conductance if the
   * contact point is off the face and tangential_tolerance is nonzero.
   */
  Real _edge_multiplier;

  bool _has_info;

  std::vector<unsigned int> _disp_vars;

  const VariableValue & _gap_distance_value;
  const VariableValue & _gap_temp_value;

  PenetrationLocator * _penetration_locator;
  const bool _warnings;

  Point & _p1;
  Point & _p2;

  /// The current \p PenetratationInfo
  const PenetrationInfo * _pinfo;

  /// The phi values on the secondary side
  const std::vector<std::vector<Real>> * _secondary_side_phi;

  /// The secondary side element (this really is a *side* element, e.g. it has
  /// dimension: mesh_dimension - 1)
  const Elem * _secondary_side;

  /// The secondary side shape index
  unsigned int _secondary_j;
};
