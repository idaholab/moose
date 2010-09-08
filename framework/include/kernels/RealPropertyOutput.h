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

#ifndef REALPROPERTYOUTPUT_H
#define REALPROPERTYOUTPUT_H

#include "Kernel.h"


// Forward Declaration
class RealPropertyOutput;

template<>
InputParameters validParams<RealPropertyOutput>();


class RealPropertyOutput : public Kernel
{
public:

  RealPropertyOutput(const std::string & name, MooseSystem & moose_system, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  std::string _prop_name;

  MaterialProperty<Real> & _prop;
};
#endif //REALPROPERTYOUTPUT_H
