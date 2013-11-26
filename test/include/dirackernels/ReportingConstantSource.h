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

#ifndef REPORTINGCONSTANTSOURCE_H
#define REPORTINGCONSTANTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

//Forward Declarations
class ReportingConstantSource;

template<>
InputParameters validParams<ReportingConstantSource>();

/**
 * A test class that uses a AuxScalarVariable to share with another
 * kernel as well as report the value via a postprocessor
 */
class ReportingConstantSource : public DiracKernel
{
public:
  ReportingConstantSource(const std::string & name, InputParameters parameters);
  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  VariableValue & _shared_var;
  std::vector<Real> _point_param;
  Point _p;
  Real _factor;
};

#endif //REPORTINGCONSTANTSOURCE_H
