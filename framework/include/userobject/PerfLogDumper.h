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

#ifndef PERFLOGDUMPER_H
#define PERFLOGDUMPER_H

#include "GeneralUserObject.h"

class PerfLogDumper;

template <>
InputParameters validParams<PerfLogDumper>();

/// Records all post processor data in a CSV file.
class PerfLogDumper : public GeneralUserObject
{
public:
  PerfLogDumper(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};
};

#endif
