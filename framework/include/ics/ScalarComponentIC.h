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

#ifndef SCALARCOMPONENTIC_H
#define SCALARCOMPONENTIC_H

#include "ScalarInitialCondition.h"

class ScalarComponentIC;

template<>
InputParameters validParams<ScalarComponentIC>();

/**
 * Initial condition to set different values on each component of scalar variable
 */
class ScalarComponentIC : public ScalarInitialCondition
{
public:
  ScalarComponentIC(const std::string & name, InputParameters parameters);
  virtual ~ScalarComponentIC();

protected:
  virtual Real value();

  std::vector<Real> _initial_values;
};

#endif /* SCALARCOMPONENTIC_H */
