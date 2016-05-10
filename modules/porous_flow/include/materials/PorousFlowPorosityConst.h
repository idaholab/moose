/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPOROSITYCONST_H
#define POROUSFLOWPOROSITYCONST_H

#include "PorousFlowPorosityUnity.h"

//Forward Declarations
class PorousFlowPorosityConst;

template<>
InputParameters validParams<PorousFlowPorosityConst>();

/**
 * Material designed to provide the porosity
 * which is assumed constant
 */
class PorousFlowPorosityConst : public PorousFlowPorosityUnity
{
public:
  PorousFlowPorosityConst(const InputParameters & parameters);

protected:
  /// constant input value of porosity
  const Real _input_porosity;

  virtual void initQpStatefulProperties();

  virtual void computeQpProperties();
};

#endif //POROUSFLOWPOROSITYCONST_H
