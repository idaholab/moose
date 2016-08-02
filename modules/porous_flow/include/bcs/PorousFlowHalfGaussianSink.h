/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWHALFGAUSSIANSINK_H
#define POROUSFLOWHALFGAUSSIANSINK_H

#include "PorousFlowSink.h"

// Forward Declarations
class PorousFlowHalfGaussianSink;

template<>
InputParameters validParams<PorousFlowHalfGaussianSink>();

/**
 * Applies a flux sink to a boundary.  The base flux
 * defined by PorousFlowSink is multiplied by a
 * _maximum*exp(-(0.5*(p - c)/_sd)^2)*_m_func for p<c
 * _maximum*_m_func for p>=c
 * This is typically used for modelling evapotranspiration
 * from the top of a groundwater model
 */
class PorousFlowHalfGaussianSink : public PorousFlowSink
{
public:

  PorousFlowHalfGaussianSink(const InputParameters & parameters);

protected:
  /// maximum of the Gaussian sink
  const Real _maximum;

  /// standard deviation of the Gaussian sink
  const Real _sd;

  /// center of the Gaussian sink
  const Real _center;

  /// Nodal pore pressure in each phase
  const MaterialProperty<std::vector<Real> > & _pp;

  /// d(Nodal pore pressure in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real> > > & _dpp_dvar;

  virtual Real multiplier();

  virtual Real dmultiplier_dvar(unsigned int pvar);
};

#endif //POROUSFLOWHALFGAUSSIANSINK_H
