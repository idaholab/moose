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

//CPPUnit includes
#include "cppunit/XmlOutputter.h"
#include "cppunit/CompilerOutputter.h"
#include "cppunit/ui/text/TestRunner.h"
#include "cppunit/extensions/TestFactoryRegistry.h"

//libMesh includes
#include "perf_log.h"

//Moose includes
#include "Moose.h"

//Moose Tests
#include "ParallelUniqueIdTest.h"
#include "LinearInterpolationTest.h"
#include "MooseArrayTest.h"
#include "ColumnMajorMatrixTest.h"
#include "ParsedFunctionTest.h"

//Elk Tests
#ifdef ELK_TEST
#include "IsotropicElasticityTensorTest.h"
#endif

#include <fstream>
#include <string>

PerfLog Moose::perf_log("CppUnit");

int main(int argc, char **argv)
{
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextTestRunner runner;
  runner.addTest(suite);

  /* Uncomment the following line for bitten testing */
  std::ofstream out("test_results.xml");

  if (argc == 2 && std::string(argv[1]) == std::string("--xml"))
    runner.setOutputter ( new CppUnit::XmlOutputter( &runner.result(), out ) );

  runner.setOutputter ( new CppUnit::CompilerOutputter( &runner.result(), std::cerr ) );

  bool wasSucessful = runner.run("", false, true, false);

  out.close();
  
  return wasSucessful ? 0 : 1;
}
