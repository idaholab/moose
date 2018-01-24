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

#ifndef FUNCTIONDIRACSOURCE_H
#define FUNCTIONDIRACSOURCE_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class FunctionDiracSource;
class Function;

template <>
InputParameters validParams<FunctionDiracSource>();

class FunctionDiracSource : public DiracKernel
{
public:
  FunctionDiracSource(const InputParameters & parameters);

  virtual void addPoints() override;

protected:
  virtual Real computeQpResidual() override;

  Function & _function;
  Point _p;
};

#endif // FunctionDiracSource_H
