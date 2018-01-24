//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef REPORTINGCONSTANTSOURCE_H
#define REPORTINGCONSTANTSOURCE_H

// Moose Includes
#include "DiracKernel.h"

// Forward Declarations
class ReportingConstantSource;

template <>
InputParameters validParams<ReportingConstantSource>();

/**
 * A test class that uses a AuxScalarVariable to share with another
 * kernel as well as report the value via a postprocessor
 */
class ReportingConstantSource : public DiracKernel
{
public:
  ReportingConstantSource(const InputParameters & parameters);
  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  VariableValue & _shared_var;
  std::vector<Real> _point_param;
  Point _p;
  Real _factor;
};

#endif // REPORTINGCONSTANTSOURCE_H
