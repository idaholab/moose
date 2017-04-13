/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GAPHEATTRANSFER_H
#define GAPHEATTRANSFER_H

#include "IntegratedBC.h"
#include "GapConductance.h"

// Forward Declarations
class GapHeatTransfer;

template <>
InputParameters validParams<GapHeatTransfer>();

/**
 * Generic gap heat transfer model, with h_gap =  h_conduction + h_contact + h_radiation
 */
class GapHeatTransfer : public IntegratedBC
{
public:
  GapHeatTransfer(const InputParameters & parameters);

  virtual void computeResidual() override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;

  virtual Real gapLength() const;
  virtual Real dgapLength(Real normalComponent) const;
  virtual Real computeSlaveFluxContribution(Real grad_t);
  virtual void computeGapValues();

  bool _gap_geometry_params_set;
  GapConductance::GAP_GEOMETRY _gap_geometry_type;

  const bool _quadrature;

  NumericVector<Number> * _slave_flux;

  const MaterialProperty<Real> & _gap_conductance;
  const MaterialProperty<Real> & _gap_conductance_dT;

  const Real _min_gap;
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

  Point _p1;
  Point _p2;
};

#endif // GAPHEATTRANSFER_H
