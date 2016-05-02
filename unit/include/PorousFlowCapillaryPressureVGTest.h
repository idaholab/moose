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

#ifndef POROUSFLOWCAPILARYPRESSUREVGTEST_H
#define POROUSFLOWCAPILARYPRESSUREVGTEST_H

//CPPUnit includes
#include "GuardedHelperMacros.h"

// Moose includes
#include "PorousFlowCapillaryPressureVG.h"

class PorousFlowCapillaryPressureVGTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE( PorousFlowCapillaryPressureVGTest );

  CPPUNIT_TEST( effTest );
  CPPUNIT_TEST( capTest );
  CPPUNIT_TEST( dcapTest );
  CPPUNIT_TEST( d2capTest );

  CPPUNIT_TEST_SUITE_END();

public:
  PorousFlowCapillaryPressureVGTest();

  void effTest();
  void capTest();
  void dcapTest();
  void d2capTest();

 private:
  Real _ep;
};

#endif  // POROUSFLOWCAPILARYPRESSUREVGTEST_H
