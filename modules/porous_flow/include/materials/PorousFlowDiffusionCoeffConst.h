/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDIFFUSIONCOEFFCONST_H
#define POROUSFLOWDIFFUSIONCOEFFCONST_H

#include "PorousFlowMaterialVectorBase.h"

class PorousFlowDiffusionCoeffConst;

template<>
InputParameters validParams<PorousFlowDiffusionCoeffConst>();

/**
 * Base class Material designed to provide the tortuosity and diffusion coefficents.
 * In this class these are constant
 */
class PorousFlowDiffusionCoeffConst : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowDiffusionCoeffConst(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Tortuosity tau_0 * tau_{alpha} for fluid phase alpha
  MaterialProperty<std::vector<Real> > & _tortuosity;

  /// Derivative of tortuosity wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real> > > & _dtortuosity_dvar;

  /// Diffusion coefficients of component k in fluid phase alpha
  MaterialProperty<std::vector<std::vector<Real> > > & _diffusion_coeff;

  /// Derivative of the diffusion coefficients wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real> > > > & _ddiffusion_coeff_dvar;

  /// Input tortuosity
  const std::vector<Real> _input_tortuosity;

  /// Input diffusion coefficients
  const std::vector<Real> _input_diffusion_coeff;
};

#endif //POROUSFLOWDIFFUSIONCOEFFCONST_H
