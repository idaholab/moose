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

#ifndef EIGENVALUEPOSTPROCESSOR_H
#define EIGENVALUEPOSTPROCESSOR_H

#include "GeneralVectorPostprocessor.h"
#include "NonlinearEigenSystem.h"

//Forward Declarations
class EigenvaluePostprocessor;

template<>
InputParameters validParams<EigenvaluePostprocessor>();

class EigenvaluePostprocessor : public GeneralVectorPostprocessor
{
public:
  EigenvaluePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  VectorPostprocessorValue & _eigen_values_real;
  VectorPostprocessorValue & _eigen_values_imag;
  NonlinearEigenSystem & _nl_eigen;
};

#endif // EIGENVALUEPOSTPROCESSOR_H
