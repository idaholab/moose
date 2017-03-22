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

#ifndef NACLFLUIDPROPERTIESTEST_H
#define NACLFLUIDPROPERTIESTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class MooseMesh;
class FEProblem;
class NaClFluidProperties;

class NaClFluidPropertiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NaClFluidPropertiesTest);

  /**
   * Verify calculation of the NACL properties the solid halite phase.
   * Density data from Brown, "The NaCl pressure standard", J. Appl. Phys., 86 (1999).
   *
   * Values for cp and enthalpy are difficult to compare against. Instead, the
   * values provided by the BrineFluidProperties UserObject were compared against
   * simple correlations, eg. from NIST sodium chloride data.
   *
   * Values for thermal conductivity from Urqhart and Bauer,
   * Experimental determination of single-crystal halite thermal conductivity,
   * diffusivity and specific heat from -75 C to 300 C, Int. J. Rock Mech.
   * and Mining Sci., 78 (2015)
   */
  CPPUNIT_TEST(halite);

  /**
   * Verify calculation of the derivatives of halite properties by comparing with finite
   * differences
   */
  CPPUNIT_TEST(derivatives);

  CPPUNIT_TEST_SUITE_END();

public:
  void registerObjects(Factory & factory);
  void buildObjects();

  void setUp();
  void tearDown();

  void halite();
  void derivatives();

private:
  MooseApp * _app;
  Factory * _factory;
  MooseMesh * _mesh;
  FEProblem * _fe_problem;
  const NaClFluidProperties * _fp;
};

#endif // NACLFLUIDPROPERTIESTEST_H
