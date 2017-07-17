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

/**
 * VectorRankTwoAux is designed to take the data in a std::vector<RankTwoTensor>
 * material property, such as provided by linear viscoelastic models, and output 
 * the value for the supplied indices.
 */
class VectorRankTwoAux : public AuxKernel
{
public:
  VectorRankTwoAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  /// The material property to extract.
  const MaterialProperty<std::vector<RankTwoTensor>> & _tensors;

  /// Index of the RankTwoTensor in the vector.
  const unsigned int _position;

  const unsigned int _i;
  const unsigned int _j;

  /// whether or not selected_qp has been set.
  const bool _has_selected_qp;
  /// value of the qp at which the property is extracted.
  const unsigned int _selected_qp;
};

#endif // VECTORRANKTWOAUX_H
