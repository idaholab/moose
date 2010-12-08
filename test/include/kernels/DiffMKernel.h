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

#ifndef DIFFMKERNEL_H
#define DIFFMKERNEL_H

#include "Kernel.h"
#include "MaterialProperty.h"

// Forward Declaration
class DiffMKernel;

template<>
InputParameters validParams<DiffMKernel>();


class DiffMKernel : public Kernel
{
public:
  DiffMKernel(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  std::string _prop_name;
  MaterialProperty<Real> & _diff;
  MaterialProperty<std::vector<Real> > & _vec_prop;
};
#endif //DIFFMKERNEL_H
