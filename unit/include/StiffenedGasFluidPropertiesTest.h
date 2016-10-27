#ifndef StiffenedGasFluidPropertiesTest_H
#define StiffenedGasFluidPropertiesTest_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class MooseMesh;
class FEProblem;
class StiffenedGasFluidProperties;


class StiffenedGasFluidPropertiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StiffenedGasFluidPropertiesTest);

  CPPUNIT_TEST(testAll);

  CPPUNIT_TEST_SUITE_END();

public:
  void registerObjects(Factory & factory);
  void buildObjects();

  void setUp();
  void tearDown();
  // test
  void testAll();

protected:
  MooseApp * _app;
  Factory * _factory;
  MooseMesh * _mesh;
  FEProblem * _fe_problem;
  const StiffenedGasFluidProperties * _fp;
};

#endif /* StiffenedGasFluidPropertiesTest_H */
