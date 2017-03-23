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
CPPUNIT_TEST_SUITE_REGISTRATION(ExpressionBuilderTest);

void
ExpressionBuilderTest::test()
{
  EBTerm x;
  x = "x";

  EBTerm y("y");
  EBTerm z("z"), u("u"), v("v"), w("w");

  EBTerm a(1), b(4), c(8.9);

  EBTerm f = pow(x, 2) + pow(y, a) + 3 * y * (-a);

  EBFunction G, H;

  // Test using functions in terms
  {
    G((x, y)) = f;
    H((x)) = -G((x, b)) + c % b;

    CPPUNIT_ASSERT(std::string(H) == "-(x^2+4^1+3*4*-1)+8.9%4");
    CPPUNIT_ASSERT(G.args() == "x,y");
  }

  // Test substitution subtleties (this would return x^x if substitution were performed
  // sequentially, which they are not.. ..because that would be dumb!)
  {
    G((x, y)) = pow(x, y);
    H((x, y, z)) = x * y * z;
    CPPUNIT_ASSERT(std::string(G) == "x^y");
    CPPUNIT_ASSERT(std::string(G((y, x))) == "y^x");
    CPPUNIT_ASSERT(std::string(H((z, y, x))) == "z*y*x");
  }

  // Test single bracket syntax
  {
    G(x, y, z) = x * y * z;
    H(u, v, w, x, y, z) = u + v + w + x + y + z;

    CPPUNIT_ASSERT(std::string(G(a, b, c)) == "1*4*8.9");
    CPPUNIT_ASSERT(std::string(H(a, b, c, a, b, c)) == "1+4+8.9+1+4+8.9");
    CPPUNIT_ASSERT(std::string(G(a, b, c) + H(a, b, c, a, b, c)) == "1*4*8.9+1+4+8.9+1+4+8.9");

    H(u, v, w, x, y, z) = u + v + w + G(y, x, x);
    CPPUNIT_ASSERT(std::string(H(a, b, c, x, y, z)) == "1+4+8.9+y*x*x");
  }

  // Test associativity
  {
    EBTerm def = x - y - z;
    EBTerm left = (x - y) - z;
    EBTerm right = x - (y - z);

    CPPUNIT_ASSERT(std::string(def) == "x-y-z");
    CPPUNIT_ASSERT(std::string(left) == "x-y-z");
    CPPUNIT_ASSERT(std::string(right) == "x-(y-z)");
  }

  // test comparison operators
  {
    EBTerm comp1 = (x < y) + (x > y);
    EBTerm comp2 = (x <= y) + (x >= y);
    EBTerm comp3 = (x == y) + (x != y);

    CPPUNIT_ASSERT(std::string(comp1) == "(x<y)+(x>y)");
    CPPUNIT_ASSERT(std::string(comp2) == "(x<=y)+(x>=y)");
    CPPUNIT_ASSERT(std::string(comp3) == "(x=y)+(x!=y)");
  }

  // test binary functions
  {
    EBTerm comp4 = atan2(x, y);
    CPPUNIT_ASSERT(std::string(comp4) == "atan2(x,y)");
  }

  // test ifexpr
  {
    EBTerm if1 = conditional(x < 2 * y, x * x + y, y * y + x);
    CPPUNIT_ASSERT(std::string(if1) == "if"
                                       "(x<2*y,x*x+y,y*y+x)");
  }

  // test temp id node stringify
  {
    EBTerm temp1;
    EBTerm temp2;
    CPPUNIT_ASSERT(std::string(temp1) != std::string(temp2));
  }

  // test substitution
  {
    // plog substitution
    EBTerm u = log(x / a);
    u.substitute(EBLogPlogSubstitution(b));
    CPPUNIT_ASSERT(std::string(u) == "plog(x/1,4)");

    // single substitution in a ternary
    EBTerm v = conditional(x < y, x * y, x / y);
    v.substitute(EBTermSubstitution(x, a));
    CPPUNIT_ASSERT(std::string(v) == "if"
                                     "(1<y,1*y,1/y)");

    // substitution list
    EBTerm w = conditional(x < y, x * y, x / y);
    EBSubstitutionRuleList s(2);
    s[0] = new EBTermSubstitution(x, y);
    s[1] = new EBTermSubstitution(y, a);
    w.substitute(s);
    CPPUNIT_ASSERT(std::string(w) == "if"
                                     "(y<1,y*1,y/1)");

    delete s[0];
    delete s[1];
  }
}
