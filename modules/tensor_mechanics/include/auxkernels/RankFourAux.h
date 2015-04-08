/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RANKFOURAUX_H
#define RANKFOURAUX_H

#include "AuxKernel.h"
#include "ElasticityTensorR4.h"

class RankFourAux;

/**
 * RankFourAux is designed to take the data in the ElasticityTensorR4 material
 * property, for example stiffness, and output the value for the
 * supplied indices.
 */

template<>
InputParameters validParams<RankFourAux>();

class RankFourAux : public AuxKernel
{
public:
  RankFourAux(const std::string & name, InputParameters parameters);

  virtual ~ RankFourAux() {}

protected:
  virtual Real computeValue();

private:
  const MaterialProperty<ElasticityTensorR4> & _tensor;
  const unsigned int _i;
  const unsigned int _j;
  const unsigned int _k;
  const unsigned int _l;
};

#endif //RANKFOURAUX_H
