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

#ifndef WATER97FLUIDPROPERTIESTEST_H
#define WATER97FLUIDPROPERTIESTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class MooseMesh;
class FEProblem;
class Water97FluidProperties;

class Water97FluidPropertiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Water97FluidPropertiesTest);

  /**
   * Verify that the correct region is provided for a given pressure and
   * temperature. Also verify that an error is thrown if pressure and temperature
   * are outside the range of validity
   */
  CPPUNIT_TEST(inRegion);

  /**
   * Verify calculation of water properties in for the boundary between regions 2 and 3
   * using the verification point (P,T) = (16.5291643 MPa, 623.15 K)
   * Revised Release on the IAPWS Industrial Formulation 1997 for the
   * Thermodynamic Properties of Water and Steam, IAPWS 2007
   */
  CPPUNIT_TEST(b23);

  /**
   * Verify calculation of water properties in region 4 (saturation line)
   * using the verification values given in Table 35 of
   * Revised Release on the IAPWS Industrial Formulation 1997 for the
   * Thermodynamic Properties of Water and Steam, IAPWS 2007
   */
  CPPUNIT_TEST(pSat);

  /**
   * Verify calculation of water properties in region 4 (saturation line)
   * using the verification values given in Table 36 of
   * Revised Release on the IAPWS Industrial Formulation 1997 for the
   * Thermodynamic Properties of Water and Steam, IAPWS 2007
   */
  CPPUNIT_TEST(TSat);

  /**
   * Verify calculation of the subregion in all 26 subregions in region 3 from
   * Revised Supplementary Release on Backward Equations for
   * Specific Volume as a Function of Pressure and Temperature v(p,T)
   * for Region 3 of the IAPWS Industrial Formulation 1997 for the
   * Thermodynamic Properties of Water and Steam
   */
  CPPUNIT_TEST(subregion3);

  /**
   * Verify calculation of the density in all 26 subregions in region 3 from
   * Revised Supplementary Release on Backward Equations for
   * Specific Volume as a Function of Pressure and Temperature v(p,T)
   * for Region 3 of the IAPWS Industrial Formulation 1997 for the
   * Thermodynamic Properties of Water and Steam
   */
  CPPUNIT_TEST(subregion3Density);

  /**
   * Verify calculation of the water properties in all regions using verification
   * data provided in IAPWS guidelines.
   * Density, enthalpy, internal energy, entropy, cp and speed of sound data from:
   * Revised Release on the IAPWS Industrial Formulation 1997 for the
   * Thermodynamic Properties of Water and Steam, IAPWS 2007.
   *
   * Viscosity data from:
   * Table 4 of Release on the IAPWS Formulation 2008 for the Viscosity of
   * Ordinary Water Substance.
   *
   * Thermal conductivity data from:
   * Table D1 of Revised Release on the IAPS Formulation 1985 for the Thermal
   * Conductivity of Ordinary Water Substance
   */
  CPPUNIT_TEST(properties);

  /**
   * Verify calculation of the derivatives in all regions by comparing with finite
   * differences
   */
  CPPUNIT_TEST(derivatives);

  CPPUNIT_TEST_SUITE_END();

public:
  void registerObjects(Factory & factory);
  void buildObjects();

  void setUp();
  void tearDown();

  void inRegion();
  void b23();
  void pSat();
  void TSat();
  void subregion3();
  void subregion3Density();
  void properties();
  void derivatives();
  void regionDerivatives(Real p, Real T, Real tol);

private:
  MooseApp * _app;
  Factory * _factory;
  MooseMesh * _mesh;
  FEProblem * _fe_problem;
  const Water97FluidProperties * _fp;
};

#endif // WATER97FLUIDPROPERTIESTEST_H
