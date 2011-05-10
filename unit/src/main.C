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

//Moose includes
#include "Moose.h"

//libMesh includes
#include "perf_log.h"

#include <fstream>
#include <string>

PerfLog Moose::perf_log("CppUnit");

int main(int argc, char **argv)
{
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextTestRunner runner;
  runner.addTest(suite);
  std::ofstream out;

  if (argc == 2 && std::string(argv[1]) == std::string("--xml")) 
  {
    runner.setOutputter ( new CppUnit::XmlOutputter( &runner.result(), out ) );
    out.open("test_results.xml");
  }

  runner.setOutputter ( new CppUnit::CompilerOutputter( &runner.result(), std::cerr ) );

  bool wasSucessful = runner.run(/*testPath=*/"", 
				 /*doWait=*/false, 
				 /*doPrintResult=*/true, 
				 /*doPrintProgress=*/false);

  if (out.is_open())
    out.close();
  
  return wasSucessful ? 0 : 1;
}
