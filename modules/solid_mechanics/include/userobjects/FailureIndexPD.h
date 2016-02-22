/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef FAILUREINDEXPD_H
#define FAILUREINDEXPD_H

#include "ElementUserObject.h"

class FailureIndexPD;

template<>
InputParameters validParams<FailureIndexPD>();

class FailureIndexPD :
  public ElementUserObject
{
public:
  FailureIndexPD(const InputParameters & parameters);

  virtual ~FailureIndexPD();

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & u );
  virtual void finalize();
  virtual Real computeFailureIndex(unsigned int nodeid) const;

protected:

  AuxiliarySystem & _aux;

  MooseVariable * _intact_bonds_var;
  MooseVariable * _total_bonds_var;

  const MaterialProperty<Real> & _bond_critical_strain; 
  const MaterialProperty<Real> & _bond_mechanic_strain; 

  VariableValue & _bond_status_old;

};

#endif // FAILUREINDEXPD_H
