/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PRESSURETM_H
#define PRESSURETM_H

#include "IntegratedBC.h"

//Forward Declarations
class Function;
class PressureTM;

template<>
InputParameters validParams<PressureTM>();

/**
 * PressureTM applies a pressure on a given boundary in the direction defined by component
 */
class PressureTM : public IntegratedBC
{
public:

  PressureTM(const std::string & name, InputParameters parameters);

  virtual ~PressureTM(){}

protected:

  virtual Real computeQpResidual();

  const int _component;

  const Real _factor;

  Function * const _function;

  const PostprocessorValue * const _postprocessor;
};

#endif //PRESSURETM_H
