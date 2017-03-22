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

#ifndef CO2FLUIDPROPERTIESTEST_H
#define CO2FLUIDPROPERTIESTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class MooseMesh;
class FEProblem;
class CO2FluidProperties;

class CO2FluidPropertiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CO2FluidPropertiesTest);

  /**
   * Verify calculation of melting pressure using experimental data from
   * Michels et al, The melting line of carbon dioxide up to 2800 atmospheres,
   * Physica 9 (1942).
   * Note that results in this reference are given in atm, but have been
   * converted to MPa here.
   * As we are comparing with experimental data, calculated values within 1% are
   * considered satisfactory.
   */
  CPPUNIT_TEST(melting);

  /**
   * Verify calculation of sublimation pressure using data from
   * Bedford et al., Recommended values of temperature for a selected set of
   * secondary reference points, Metrologia 20 (1984).
   */
  CPPUNIT_TEST(sublimation);

  /**
   * Verify calculation of vapor pressure, vapor density and saturated liquid
   * density using experimental data from
   * Duschek et al., Measurement and correlation of the (pressure, density, temperature)
   * relation of cabon dioxide II. Saturated-liquid and saturated-vapor densities and
   * the vapor pressure along the entire coexstance curve, J. Chem. Thermo. 22 (1990).
   * As we are comparing with experimental data, calculated values within 1% are
   * considered satisfactory.
   */
  CPPUNIT_TEST(vapor);

  /**
   * Verify calculation of partial density at infinite dilution using data from
   * Hnedkovsky et al., Volumes of aqueous solutions of CH4, CO2, H2S, and NH3
   * at temperatures from 298.15 K to 705 K and pressures to 35 MPa,
   * J. Chem. Thermo. 28, 1996.
   * As we are comparing with experimental data, calculated values within 5% are
   * considered satisfactory.
   */
  CPPUNIT_TEST(partialDensity);

  /**
   * Verify calculation of Henry's constant using data from
   * Guidelines on the Henry's constant and vapour liquid distribution constant
   * for gases in H20 and D20 at high temperatures, IAPWS (2004).
   */
  CPPUNIT_TEST(henry);

  /**
   * Verify calculation of thermal conductivity using data from
   * Scalabrin et al., A Reference Multiparameter Thermal Conductivity Equation
   * for Carbon Dioxide with an Optimized Functional Form,
   * J. Phys. Chem. Ref. Data 35 (2006)
   */
  CPPUNIT_TEST(thermalConductivity);

  /**
   * Verify calculation of viscosity using data from
   * Fenghour et al., The viscosity of carbon dioxide,
   * J. Phys. Chem. Ref. Data, 27, 31-44 (1998)
   */
  CPPUNIT_TEST(viscosity);

  /**
   * Verify calculation of thermophysical properties of CO2 from the Span and
   * Wagner EOS using verification data provided in
   * A New Equation of State for Carbon Dioxide Covering the Fluid Region from
   * the Triple-Point Temperature to 1100K at Pressures up to 800 MPa,
   * J. Phys. Chem. Ref. Data, 25 (1996)
   */
  CPPUNIT_TEST(propertiesSW);

  /**
   * Verify calculation of the derivatives of all properties by comparing with finite
   * differences
   */
  CPPUNIT_TEST(derivatives);

  CPPUNIT_TEST_SUITE_END();

public:
  void registerObjects(Factory & factory);
  void buildObjects();

  void setUp();
  void tearDown();

  void melting();
  void sublimation();
  void vapor();
  void partialDensity();
  void henry();
  void thermalConductivity();
  void viscosity();
  void propertiesSW();
  void derivatives();

private:
  MooseApp * _app;
  Factory * _factory;
  MooseMesh * _mesh;
  FEProblem * _fe_problem;
  const CO2FluidProperties * _fp;
};

#endif // CO2FLUIDPROPERTIESTEST_H
