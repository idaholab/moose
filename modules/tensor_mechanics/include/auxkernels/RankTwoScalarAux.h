/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RANKTWOSCALARAUX_H
#define RANKTWOSCALARAUX_H

#include "AuxKernel.h"
#include "RankTwoTensor.h"

class RankTwoScalarAux;

/**
 * RankTwoScalarAux provides scalar values of RankTwoTensor
 * Currently supported are VonMisesStress, EquivalentPlasticStrain, Hydrostatic, L2norm,
 * MaxPrincipal, MidPrincipal, MinPrincipal
 */

template<>
InputParameters validParams<RankTwoScalarAux>();

class RankTwoScalarAux : public AuxKernel
{
public:
  RankTwoScalarAux(const InputParameters & parameters);
  virtual ~RankTwoScalarAux() {}

protected:
  virtual Real computeValue();

  const MaterialProperty<RankTwoTensor> & _tensor;

  /**
   * Determines the information to be extracted from
   * the tensor.  Eg, vonMisesStress, EquivalentPlasticStrain,
   * L2norm, MaxPrincipal eigenvalue, etc.
   */
  MooseEnum _scalar_type;

  /// whether or not selected_qp has been set
  const bool _has_selected_qp;

  /// The std::vector will be evaluated at this quadpoint only
  const unsigned int _selected_qp;

  /**
   * Calculates the eigenvalues of the tensor
   * at the given quadpoint, and returns the
   * one of interest, depending on _scalar_type
   *
   * @param qp the quadpoint
   * @return the eigenvalue of interest
   */
  virtual Real calcEigenValues(unsigned int qp);
};

#endif //RANKTWOSCALARAUX_H
