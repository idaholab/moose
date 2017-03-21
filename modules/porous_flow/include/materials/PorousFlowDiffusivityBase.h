/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDIFFUSIVITYBASE_H
#define POROUSFLOWDIFFUSIVITYBASE_H

#include "PorousFlowMaterialVectorBase.h"

class PorousFlowDiffusivityBase;

template <>
InputParameters validParams<PorousFlowDiffusivityBase>();

/// Base class Material designed to provide the tortuosity and diffusion coefficents
class PorousFlowDiffusivityBase : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowDiffusivityBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Tortuosity tau_0 * tau_{alpha} for fluid phase alpha
  MaterialProperty<std::vector<Real>> & _tortuosity;

  /// Derivative of tortuosity wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<Real>>> & _dtortuosity_dvar;

  /// Diffusion coefficients of component k in fluid phase alpha
  MaterialProperty<std::vector<std::vector<Real>>> & _diffusion_coeff;

  /// Derivative of the diffusion coefficients wrt PorousFlow variables
  MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _ddiffusion_coeff_dvar;

  /// Input diffusion coefficients
  const std::vector<Real> _input_diffusion_coeff;
};

#endif // POROUSFLOWDIFFUSIVITYBASE_H
