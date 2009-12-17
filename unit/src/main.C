//CPPUnit includes
#include "cppunit/XmlOutputter.h"
#include "cppunit/CompilerOutputter.h"
#include "cppunit/ui/text/TestRunner.h"
#include "cppunit/extensions/TestFactoryRegistry.h"

//libMesh includes
#include "perf_log.h"

//Moose includes
#include "ColumnMajorMatrixTest.h"
#include "Moose.h"

PerfLog Moose::perf_log("CppUnit");

int main(int argc, char **argv)
{
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);

  /* Uncomment the following line for bitten testing */
  runner.setOutputter ( new CppUnit::XmlOutputter( &runner.result(), std::cerr ) );
  
  /* Uncomment the following line for CLI testing */
  //runner.setOutputter ( new CppUnit::CompilerOutputter( &runner.result(), std::cerr ) );

  bool wasSucessful = runner.run();
  
  return wasSucessful ? 0 : 1;
}
