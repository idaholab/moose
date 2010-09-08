/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MTBC_H
#define MTBC_H

#include "BoundaryCondition.h"


//Forward Declarations
class MTBC;

template<>
InputParameters validParams<MTBC>();

/**
 * Implements a simple Neumann BC with material where grad(u)=value on the boundary.
 */
class MTBC : public BoundaryCondition
{
public:
  MTBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~MTBC() {}

protected:
  virtual Real computeQpResidual();
  
private:
  /**
   * Value of grad(u) on the boundary.
   */
  Real _value;
  std::string _prop_name;
  MaterialProperty<Real> & _mat;
};

#endif //NEUMANNBC_H
