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

#ifndef NODALNORMALBC_H
#define NODALNORMALBC_H

#include "NodalBC.h"

class NodalNormalBC;

template<>
InputParameters validParams<NodalNormalBC>();

/**
 * This is a base class to enforce strong boundary condition with a normal defined at a node
 *
 * NOTE: This class will not compute the normal! It is computed in a user object subsystem.
 */
class NodalNormalBC : public NodalBC
{
public:
  NodalNormalBC(const std::string & name, InputParameters parameters);
  virtual ~NodalNormalBC();

  virtual void computeResidual(NumericVector<Number> & residual);

protected:
  NumericVector<Real> & _nx;
  NumericVector<Real> & _ny;
  NumericVector<Real> & _nz;

  /// Normal at the node (it is pre-computed by user object subsystem)
  Point _normal;
};


#endif /* NODALNORMALBC_H */
