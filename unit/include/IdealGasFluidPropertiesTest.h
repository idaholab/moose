#ifndef IDEALGASFLUIDPROPERTIESTEST_H
#define IDEALGASFLUIDPROPERTIESTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class MooseMesh;
class FEProblem;
class IdealGasFluidProperties;


class IdealGasFluidPropertiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IdealGasFluidPropertiesTest);

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
  const IdealGasFluidProperties * _fp;
};

#endif /* IDEALGASFLUIDPROPERTIESTEST_H */
