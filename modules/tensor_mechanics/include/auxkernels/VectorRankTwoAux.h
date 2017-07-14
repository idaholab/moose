/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef VECTORRANKTWOAUX_H
#define VECTORRANKTWOAUX_H

#include "AuxKernel.h"
#include "RankTwoTensor.h"

class VectorRankTwoAux;

template <>
InputParameters validParams<VectorRankTwoAux>();

class VectorRankTwoAux : public AuxKernel
{
public:
  VectorRankTwoAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  const MaterialProperty<std::vector<RankTwoTensor>> & _tensors;
  const unsigned int _position;
  const unsigned int _i;
  const unsigned int _j;

  const bool _has_selected_qp;
  const unsigned int _selected_qp;
};

#endif // VECTORRANKTWOAUX_H
