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

// Tutorial Includes
#include "DarcyThermoMechApp.h"

// Moose Includes
#include "MooseInit.h"
#include "Factory.h"
#include "AppFactory.h"

// Google Test includes
#include "gtest/gtest.h"

PerfLog Moose::perf_log("gtest");

GTEST_API_ int
main(int argc, char ** argv)
{
  // gtest removes (only) its args from argc and argv - so this must be before MooseInit
  testing::InitGoogleTest(&argc, argv);

  MooseInit init(argc, argv);
  registerApp(DarcyThermoMechApp);
  Moose::_throw_on_error = true;

  return RUN_ALL_TESTS();
}
