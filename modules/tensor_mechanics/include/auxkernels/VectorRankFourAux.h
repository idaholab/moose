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

/**
 * VectorRankFourAux is designed to take the data in a std::vector<RankFourTensor>
 * material property, such as provided by linear viscoelastic models, and output
 * the value for the supplied indices.
 */
class VectorRankFourAux : public AuxKernel
{
public:
  VectorRankFourAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  /// The material property to extract.
  const MaterialProperty<std::vector<RankFourTensor>> & _tensors;

  /// Index of the RankTwoTensor in the vector.
  const unsigned int _position;

  const unsigned int _i;
  const unsigned int _j;
  const unsigned int _k;
  const unsigned int _l;

  /// whether or not selected_qp has been set.
  const bool _has_selected_qp;
  /// value of the qp at which the property is extracted.
  const unsigned int _selected_qp;
};

#endif // VECTORRANKFOURAUX_H
