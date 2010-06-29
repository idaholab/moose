#include "FunctorTest.h"

//Moose includes
#include "Functor.h"

CPPUNIT_TEST_SUITE_REGISTRATION( FunctorTest );

void
FunctorTest::basicConstructor()
{
  //test constructor with no additional variables
  Functor f( "x + 1.5*y + 2 * z + t/4" );
  CPPUNIT_ASSERT( f(4, 1, 2, 3) == 11 );
}

void
FunctorTest::advancedConstructor()
{
  //test the constructor with one variable
  std::vector<std::string> one_var(1);
  one_var[0] = "q";

  Functor f("x + y + q", one_var );
  f.getVarAddr("q") = 4;
  CPPUNIT_ASSERT( f(0, 1, 2) == 7 );

  //test the constructor with three variables
  std::vector<std::string> three_vars(3);
  three_vars[0] = "q";
  three_vars[1] = "w";
  three_vars[2] = "r";

  Functor f2("r*x + y/w + q", three_vars );
  f2.getVarAddr("q") = 4;
  f2.getVarAddr("w") = 2;
  f2.getVarAddr("r") = 1.5;
  CPPUNIT_ASSERT( f2(0, 2, 4) == 9 );

  //test the constructor with one variable that's set
  std::vector<Real> one_val(1);
  one_val[0] = 2.5;

  Functor f3("q*x", one_var, one_val);
  CPPUNIT_ASSERT( f3(0,2) == 5 );

  //test the constructor with three variables, two that are set
  std::vector<Real> two_vals(2);
  two_vals[0] = 1.5;
  two_vals[1] = 1;

  Functor f4("q*x + y/r + w", three_vars, two_vals);
  f4.getVarAddr("r") = 2;
  CPPUNIT_ASSERT( f4(0, 2, 4) == 6 );
  f4.getVarAddr("r") = 4;
  CPPUNIT_ASSERT( f4(0, 2, 4) == 5 );
}

void
FunctorTest::testVariables()
{
  //a lot of this functionality is tested in advancedConstructor as well
  //test one variable, make sure we can change it by the reference any time
  std::vector<std::string> one_var(1);
  one_var[0] = "q";

  Functor f("x + y + q", one_var );
  Real & q = f.getVarAddr("q");
  q = 4;
  CPPUNIT_ASSERT( f(0, 1, 2) == 7 );
  q = 2;
  CPPUNIT_ASSERT( f(0, 1, 2) == 5 );
  q = -4;
  CPPUNIT_ASSERT( f(0, 1, 2) == -1 );

  //test three variables, test updating them randomly
  std::vector<std::string> three_vars(3);
  three_vars[0] = "q";
  three_vars[1] = "w";
  three_vars[2] = "r";

  Functor f2("r*x + y/w + q", three_vars );
  Real & q2 = f2.getVarAddr("q");
  Real & w2 = f2.getVarAddr("w");
  Real & r2 = f2.getVarAddr("r");
  q2 = 4; w2 = 2; r2 = 1.5;
  CPPUNIT_ASSERT( f2(0, 2, 4) == 9 );
  q2 = 1; w2 = 4; r2 = 2.5;
  CPPUNIT_ASSERT( f2(0, 2, 4) == 7 );
  q2 = 2;
  CPPUNIT_ASSERT( f2(0, 2, 4) == 8 );
  w2 = 3;
  CPPUNIT_ASSERT( f2(0, 2, 6) == 9 );
}

void
FunctorTest::testConstants()
{
  //this functions tests that pi and e get correctly substituted
  //it also tests built in functions of the function parser
  Functor f("log(e) + x");
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 2, f(0,1), 0.0000001 );

  Functor f2("sin(pi*x)");
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0, f2(0,1), 0.0000001 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1, f2(0,0.5), 0.0000001 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -1, f2(0,1.5), 0.0000001 );
}
