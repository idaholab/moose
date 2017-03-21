/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWDIFFUSIVITYCONST_H
#define POROUSFLOWDIFFUSIVITYCONST_H

#include "PorousFlowDiffusivityBase.h"

class PorousFlowDiffusivityConst;

template <>
InputParameters validParams<PorousFlowDiffusivityConst>();

/// Material designed to provide constant tortuosity and diffusion coefficents
class PorousFlowDiffusivityConst : public PorousFlowDiffusivityBase
{
public:
  PorousFlowDiffusivityConst(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Input tortuosity
  const std::vector<Real> _input_tortuosity;
};

#endif // POROUSFLOWDIFFUSIVITYCONST_H
