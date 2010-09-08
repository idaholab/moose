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

#ifndef CONVECTION_H
#define CONVECTION_H

#include "Kernel.h"

//Forward Declarations
class Convection;

template<>
InputParameters validParams<Convection>();

class Convection : public Kernel
{
public:

  Convection(const std::string & name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  /**
   * Coupled things come through as VaribleXYZ _references_.
   *
   * Since this is a reference it MUST be set in the Initialization List of the
   * constructor!
   */
  VariableGradient & _velocity_vector;
};

#endif //CONVECTION_H
