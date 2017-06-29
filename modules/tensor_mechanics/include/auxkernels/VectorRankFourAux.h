/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef VECTORRANKFOURAUX_H
#define VECTORRANKFOURAUX_H

#include "AuxKernel.h"
#include "RankFourTensor.h"

class VectorRankFourAux;

template <>
InputParameters validParams<VectorRankFourAux>();

class VectorRankFourAux : public AuxKernel
{
public:
  VectorRankFourAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  const MaterialProperty<std::vector<RankFourTensor>> & _tensors;
  const unsigned int _position;
  const unsigned int _i;
  const unsigned int _j;
  const unsigned int _k;
  const unsigned int _l;

  const bool _has_selected_qp;
  const unsigned int _selected_qp;
};

#endif // VECTORRANKFOURAUX_H
