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

#include "ExpressionBuilderTest.h"

// This macro declares a static variable whose construction
// causes a test suite factory to be inserted in a global registry
// of such factories.
CPPUNIT_TEST_SUITE_REGISTRATION( ExpressionBuilderTest );


void ExpressionBuilderTest::test()
{
  EBTerm x;
  x = "x";

  EBTerm y("y");

  EBTerm a(1), b(4), c(8.9);

  EBTerm f = pow(x,2) + pow(y,a) + 3 * y * (-a);

  EBFunction G, H;

  // Test using functions in terms
  G((x,y)) = f;
  H((x)) = -G((x,b)) + c;

  CPPUNIT_ASSERT( std::string(H) == "-(x^2+4^1+3*4*-1)+8.9" );
  CPPUNIT_ASSERT( G.args() == "x,y" );

  // Test substitution subtleties (this would return x^x if substitution were performed sequentially, which they are not.. ..because that would be dumb!)
  G((x,y)) = pow(x,y);
  CPPUNIT_ASSERT( std::string(G) == "x^y" );
  CPPUNIT_ASSERT( std::string(G((y,x))) == "y^x" );
}
