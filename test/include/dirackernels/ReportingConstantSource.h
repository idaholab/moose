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
 * TOOD
 */
class ReportingConstantSource : public DiracKernel
{
public:
  ReportingConstantSource(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  Real _value;
  std::vector<Real> _point_param;
  Point _p;

  PostprocessorValue & _reporter;
};

#endif //REPORTINGCONSTANTSOURCE_H
