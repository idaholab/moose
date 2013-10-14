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

#ifndef REPORTINGCONSTANTSOURCEREPORTABLE_H
#define REPORTINGCONSTANTSOURCEREPORTABLE_H

// Moose Includes
#include "DiracKernel.h"

//Forward Declarations
class ReportingConstantSourceReportable;

template<>
InputParameters validParams<ReportingConstantSourceReportable>();

/**
 * A class for testing the Reportable interface, test mimics the
 * dirackernels/total_flux. The gold file from this test was directly copied
 * to the gold for this directory
 */
class ReportingConstantSourceReportable : public DiracKernel
{
public:
  ReportingConstantSourceReportable(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  Real _value;
  std::vector<Real> _point_param;
  Point _p;
  ReportableValue & _reporter;
};

#endif //REPORTINGCONSTANTSOURCEREPORTABLE_H
