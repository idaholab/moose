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

#ifndef EIGENKERNEL_H
#define EIGENKERNEL_H

#include "Kernel.h"

//Forward Declarations
class EigenKernel;
class MooseEigenSystem;

template<>
InputParameters validParams<EigenKernel>();

/**
 * The behavior of this kernel is controlled by one problem-wise global parameter
 *    eigen_on_current - bool, to indicate if this kernel is operating on the current solution or old solution
 * This kernel also obtain the postprocessor for eigenvalue by one problem-wise global parameter
 *    eigen_postprocessor - string, the name of the postprocessor to obtain the eigenvalue
 */
class EigenKernel :
  public Kernel
{
public:
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int /*jvar*/) override {}
  virtual void computeOffDiagJacobianScalar(unsigned int /*jvar*/) override {}

  EigenKernel(const InputParameters & parameters);
  virtual bool enabled() override;

protected:
  virtual Real computeQpJacobian() override;

  /// flag for as an eigen kernel or a normal kernel
  bool _eigen;

  /// EigenKernel always lives in EigenSystem
  MooseEigenSystem * _eigen_sys;

  /**
   * A pointer to the eigenvalue that is stored in a postprocessor
   * This is a pointer so that the method for retrieval (old vs current) may be changed.
   */
  const Real * _eigenvalue;
};

#endif //EIGENKERNEL_H
