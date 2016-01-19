/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACBulk.h"

#ifndef ACGRGRPOLYCONSTRAINT_H
#define ACGRGRPOLYCONSTRAINT_H

//Forward Declarations
class ACGrGrPolyConstraint;

template<>
InputParameters validParams<ACGrGrPolyConstraint>();

/**
 * This kernel calculates a penalty based on the square of the deviation of the
 * order parameter sum deviating from unity.
 */
class ACGrGrPolyConstraint : public ACBulk<Real>
{
public:
  ACGrGrPolyConstraint(const InputParameters & parameters);

protected:
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const unsigned int _ncrys;

  std::vector<VariableValue *> _vals;
  std::vector<unsigned int> _vals_var;

  const Real _penalty;
};

#endif //ACGRGRPOLYCONSTRAINT_H
