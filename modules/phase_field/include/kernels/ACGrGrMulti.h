/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ACGRGRMULTI_H
#define ACGRGRMULTI_H

#include "ACGrGrBase.h"

// Forward Declarations
class ACGrGrMulti;

template <>
InputParameters validParams<ACGrGrMulti>();

/**
 * This kernel calculates the residual for grain growth for a multi-phase,
 * poly-crystal system. A list of material properties needs to be supplied for the gammas
 * (prefactors of the cross-terms between order parameters).
 */
class ACGrGrMulti : public ACGrGrBase
{
public:
  ACGrGrMulti(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Names of gammas for each order parameter
  std::vector<MaterialPropertyName> _gamma_names;
  unsigned int _num_j;

  /// Values of gammas for each order parameter
  std::vector<const MaterialProperty<Real> *> _prop_gammas;
};

#endif // ACGRGRMULTI_H
