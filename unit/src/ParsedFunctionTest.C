#include "ParsedFunctionTest.h"

//Moose includes
#include "MooseSystem.h"
#include "InputParameters.h"
#include "ParsedFunction.h"

CPPUNIT_TEST_SUITE_REGISTRATION( ParsedFunctionTest );

void
ParsedFunctionTest::basicConstructor()
{
  MooseSystem m;
  InputParameters params = validParams<ParsedFunction>();

  //test constructor with no additional variables
  params.set<std::string>("function") = std::string("x + 1.5*y + 2 * z + t/4");
  ParsedFunction f("test", m, params);
  CPPUNIT_ASSERT(f.value(4, 1, 2, 3) == 11);
}

void
ParsedFunctionTest::advancedConstructor()
{
  MooseSystem m;

  //test the constructor with one variable
  std::vector<std::string> one_var(1);
  one_var[0] = "q";

  InputParameters params = validParams<ParsedFunction>();
  params.set<std::string>("function") = "x + y + q";
  params.set<std::vector<std::string> >("vars") = one_var;

  ParsedFunction f("test", m, params);
  f.getVarAddr("q") = 4;
  CPPUNIT_ASSERT( f.value(0, 1, 2) == 7 );

  //test the constructor with three variables
  std::vector<std::string> three_vars(3);
  three_vars[0] = "q";
  three_vars[1] = "w";
  three_vars[2] = "r";

  InputParameters params2 = validParams<ParsedFunction>();
  params2.set<std::string>("function") = "r*x + y/w + q";
  params2.set<std::vector<std::string> >("vars") = three_vars;

  ParsedFunction f2("test", m, params2);
  f2.getVarAddr("q") = 4;
  f2.getVarAddr("w") = 2;
  f2.getVarAddr("r") = 1.5;
  CPPUNIT_ASSERT( f2.value(0, 2, 4) == 9 );

  //test the constructor with one variable that's set
  std::vector<Real> one_val(1);
  one_val[0] = 2.5;

  InputParameters params3 = validParams<ParsedFunction>();
  params3.set<std::string>("function") = "q*x";
  params3.set<std::vector<std::string> >("vars") = one_var;
  params3.set<std::vector<Real> >("vals") = one_val;

  ParsedFunction f3("test", m, params3);
  CPPUNIT_ASSERT( f3.value(0,2) == 5 );

  //test the constructor with three variables, two that are set
  std::vector<Real> two_vals(2);
  two_vals[0] = 1.5;
  two_vals[1] = 1;

  InputParameters params4 = validParams<ParsedFunction>();
  params4.set<std::string>("function") = "q*x + y/r + w";
  params4.set<std::vector<std::string> >("vars") = three_vars;
  params4.set<std::vector<Real> >("vals") = two_vals;

  ParsedFunction f4("test", m, params4);
  f4.getVarAddr("r") = 2;
  CPPUNIT_ASSERT( f4.value(0, 2, 4) == 6 );
  f4.getVarAddr("r") = 4;
  CPPUNIT_ASSERT( f4.value(0, 2, 4) == 5 );
}

void
ParsedFunctionTest::testVariables()
{
  MooseSystem m;

  //a lot of this functionality is tested in advancedConstructor as well
  //test one variable, make sure we can change it by the reference any time
  std::vector<std::string> one_var(1);
  one_var[0] = "q";

  InputParameters params = validParams<ParsedFunction>();
  params.set<std::string>("function") = "x + y + q";
  params.set<std::vector<std::string> >("vars") = one_var;

  ParsedFunction f("test", m, params);
  Real & q = f.getVarAddr("q");
  q = 4;
  CPPUNIT_ASSERT( f.value(0, 1, 2) == 7 );
  q = 2;
  CPPUNIT_ASSERT( f.value(0, 1, 2) == 5 );
  q = -4;
  CPPUNIT_ASSERT( f.value(0, 1, 2) == -1 );

  //test three variables, test updating them randomly
  std::vector<std::string> three_vars(3);
  three_vars[0] = "q";
  three_vars[1] = "w";
  three_vars[2] = "r";

  InputParameters params2 = validParams<ParsedFunction>();
  params2.set<std::string>("function") = "r*x + y/w + q";
  params2.set<std::vector<std::string> >("vars") = three_vars;

  ParsedFunction f2("test", m, params2);
  Real & q2 = f2.getVarAddr("q");
  Real & w2 = f2.getVarAddr("w");
  Real & r2 = f2.getVarAddr("r");
  q2 = 4; w2 = 2; r2 = 1.5;
  CPPUNIT_ASSERT( f2.value(0, 2, 4) == 9 );
  q2 = 1; w2 = 4; r2 = 2.5;
  CPPUNIT_ASSERT( f2.value(0, 2, 4) == 7 );
  q2 = 2;
  CPPUNIT_ASSERT( f2.value(0, 2, 4) == 8 );
  w2 = 3;
  CPPUNIT_ASSERT( f2.value(0, 2, 6) == 9 );
}

void
ParsedFunctionTest::testConstants()
{
  MooseSystem m;

  //this functions tests that pi and e get correctly substituted
  //it also tests built in functions of the function parser
  InputParameters params = validParams<ParsedFunction>();
  params.set<std::string>("function") = "log(e) + x";

  ParsedFunction f("test", m, params);
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 2, f.value(0,1), 0.0000001 );

  InputParameters params2 = validParams<ParsedFunction>();
  params2.set<std::string>("function") = "sin(pi*x)";

  ParsedFunction f2("test", m, params2);
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 0, f2.value(0,1), 0.0000001 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( 1, f2.value(0,0.5), 0.0000001 );
  CPPUNIT_ASSERT_DOUBLES_EQUAL( -1, f2.value(0,1.5), 0.0000001 );
}
