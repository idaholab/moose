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

#ifndef TESTINITIALSETUPTRANSFER_H
#define TESTINITIALSETUPTRANSFER_H

#include "MultiAppProjectionTransfer.h"

class TestInitialSetupTransfer;

template<>
InputParameters validParams<TestInitialSetupTransfer>();

/**
 * Project values from one domain to another
 */
class TestInitialSetupTransfer : public MultiAppProjectionTransfer
{
public:
  TestInitialSetupTransfer(const std::string & name, InputParameters parameters);
  virtual ~TestInitialSetupTransfer();

  virtual void execute();

  virtual void initialSetup();
};


#endif
