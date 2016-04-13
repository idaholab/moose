/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIAL2PHASEPS_VG_H
#define PORFLOWMATERIAL2PHASEPS_VG_H

#include "PorousFlowMaterial2PhasePS.h"
#include "PorousFlowCapillaryPressureVG.h"

//Forward Declarations
class PorousFlowMaterial2PhasePS_VG;

template<>
InputParameters validParams<PorousFlowMaterial2PhasePS_VG>();

/**
 * Material designed to calculate fluid-phase porepressures at nodes
 */
class PorousFlowMaterial2PhasePS_VG : public PorousFlowMaterial2PhasePS
{
public:
  PorousFlowMaterial2PhasePS_VG(const InputParameters & parameters);

protected:

  /**
   * Capillary pressure as a function of saturation.
   * Default is constant capillary pressure = 0.0.
   * Over-ride in derived classes to implement other capillary pressure forulations
   *
   * @param saturation saturation
   * @return capillary pressure
   */
  virtual Real capillaryPressure(Real saturation) const;

  /**
   * Derivative of capillary pressure wrt to saturation.
   * Default = 0 for constant capillary pressure.
   * Over-ride in derived classes to implement other capillary pressure forulations
   *
   * @param saturation saturation (Pa)
   * @return derivative of capillary pressure wrt saturation
   */
  virtual Real dCapillaryPressure_dS(Real pressure) const;

  /**
   * Second derivative of capillary pressure wrt to saturation.
   * Default = 0 for constant capillary pressure.
   * Over-ride in derived classes to implement other capillary pressure forulations
   *
   * @param saturation saturation (Pa)
   * @return second derivative of capillary pressure wrt saturation
   */
  virtual Real d2CapillaryPressure_dS2(Real pressure) const;

  /// Liquid residual saturation
  Real _sat_lr;
  /// Liquid phase fully saturated saturation
  Real _sat_ls;
  /// van Genuchten exponent m
  Real _m;
  /// Maximum capillary pressure
  Real _pc_max;
  /// van Genuchten capillary pressure coefficient
  Real _p0;
};

#endif //PORFLOWMATERIAL2PHASEPS_VG_H
