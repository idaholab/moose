/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef BONDSTATUSPDAUX_H
#define BONDSTATUSPDAUX_H

#include "AuxKernel.h"

//Forward Declarations
class BondStatusPDAux;

template<>
InputParameters validParams<BondStatusPDAux>();

class BondStatusPDAux : public AuxKernel
{
public:

  BondStatusPDAux(const InputParameters & parameters);

  virtual ~BondStatusPDAux() {}

protected:

  virtual Real computeValue();

  const MaterialProperty<Real> & _bond_critical_strain;
  const MaterialProperty<Real> & _bond_mechanic_strain;

  VariableValue & _bond_status_old;

};

#endif //BONDSTATUSPDAUX_H
