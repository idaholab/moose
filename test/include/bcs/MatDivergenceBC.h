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
#ifndef MATDIVERGENCEBC_H
#define MATDIVERGENCEBC_H

#include "DivergenceBC.h"

class MatDivergenceBC;

template <>
InputParameters validParams<MatDivergenceBC>();

/**
 * Extends DivergenceBC by multiplication of material property
 */
class MatDivergenceBC : public DivergenceBC
{
public:
  MatDivergenceBC(const InputParameters & parameters);
  virtual ~MatDivergenceBC();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const MaterialProperty<Real> & _mat;
};

#endif /* MATDIVERGENCEBC_H */
