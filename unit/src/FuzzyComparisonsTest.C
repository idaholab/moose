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

#include "MooseUtils.h"
#include "FuzzyComparisonsTest.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION(FuzzyComparisonsTest);

void
FuzzyComparisonsTest::fuzzyFunctions()
{
  libMesh::Real tol = libMesh::TOLERANCE * libMesh::TOLERANCE;
  libMesh::Real a = 1.0;
  libMesh::Real aplus = a + 0.5 * tol;
  libMesh::Real aminus = a - 0.5 * tol;
  libMesh::Real aplus2 = a + 1.5 * tol;
  libMesh::Real aminus2 = a - 1.5 * tol;

  libMesh::Real b = 1000.0;
  libMesh::Real rtol = tol * 2.0 * b;
  libMesh::Real bplus = b + 0.5 * rtol;
  libMesh::Real bminus = b - 0.5 * rtol;
  libMesh::Real bplus2 = b + 1.5 * rtol;
  libMesh::Real bminus2 = b - 1.5 * rtol;
  libMesh::Real zero = 0.0;

  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyEqual(zero, zero));
  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyEqual(tol, tol));
  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyEqual(a, aplus));
  CPPUNIT_ASSERT(!MooseUtils::absoluteFuzzyEqual(a, aplus2));

  CPPUNIT_ASSERT(!MooseUtils::absoluteFuzzyGreaterThan(aplus, a));
  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyGreaterThan(aplus2, a));

  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyGreaterEqual(aplus, a));
  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyGreaterEqual(aminus, a));
  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyGreaterEqual(aplus2, a));
  CPPUNIT_ASSERT(!MooseUtils::absoluteFuzzyGreaterEqual(aminus2, a));

  CPPUNIT_ASSERT(!MooseUtils::absoluteFuzzyLessThan(aminus, a));
  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyLessThan(aminus2, a));

  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyLessEqual(aminus, a));
  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyLessEqual(aplus, a));
  CPPUNIT_ASSERT(MooseUtils::absoluteFuzzyLessEqual(aminus2, a));
  CPPUNIT_ASSERT(!MooseUtils::absoluteFuzzyLessEqual(aplus2, a));

  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyEqual(zero, zero));
  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyEqual(tol, tol));
  CPPUNIT_ASSERT(!MooseUtils::relativeFuzzyEqual(zero, tol));
  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyEqual(b, bplus));
  CPPUNIT_ASSERT(!MooseUtils::relativeFuzzyEqual(b, bplus2));

  CPPUNIT_ASSERT(!MooseUtils::relativeFuzzyGreaterThan(bplus, b));
  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyGreaterThan(bplus2, b));

  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyGreaterEqual(bplus, b));
  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyGreaterEqual(bminus, b));
  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyGreaterEqual(bplus2, b));
  CPPUNIT_ASSERT(!MooseUtils::relativeFuzzyGreaterEqual(bminus2, b));

  CPPUNIT_ASSERT(!MooseUtils::relativeFuzzyLessThan(bminus, b));
  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyLessThan(bminus2, b));

  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyLessEqual(bminus, b));
  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyLessEqual(bplus, b));
  CPPUNIT_ASSERT(MooseUtils::relativeFuzzyLessEqual(bminus2, b));
  CPPUNIT_ASSERT(!MooseUtils::relativeFuzzyLessEqual(bplus2, b));
}
