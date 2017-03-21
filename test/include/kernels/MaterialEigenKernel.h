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

#ifndef MATERIALEIGENKERNEL_H
#define MATERIALEIGENKERNEL_H

#include "EigenKernel.h"

// Forward Declarations
class MaterialEigenKernel;

template <>
InputParameters validParams<MaterialEigenKernel>();

class MaterialEigenKernel : public EigenKernel
{
public:
  MaterialEigenKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  std::string _propname;
  const MaterialProperty<Real> & _mat;
};

#endif // MATERIALEIGENKERNEL_H
