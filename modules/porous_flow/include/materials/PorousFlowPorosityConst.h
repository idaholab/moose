/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPOROSITYCONST_H
#define POROUSFLOWPOROSITYCONST_H

#include "PorousFlowPorosityBase.h"

//Forward Declarations
class PorousFlowPorosityConst;

template<>
InputParameters validParams<PorousFlowPorosityConst>();

/**
 * Material to provide a constant porosity
 */
class PorousFlowPorosityConst : public PorousFlowPorosityBase
{
public:
  PorousFlowPorosityConst(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  virtual void computeQpProperties() override;

  /// constant input value of porosity
  const Real _input_porosity;
};

#endif //POROUSFLOWPOROSITYCONST_H
