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

#ifndef BRINEFLUIDPROPERTIESTEST_H
#define BRINEFLUIDPROPERTIESTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class MooseMesh;
class FEProblem;
class BrineFluidProperties;
class SinglePhaseFluidPropertiesPT;

class BrineFluidPropertiesTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrineFluidPropertiesTest);

  /**
   * Verify calculation of brine vapor pressure using data from
   * Haas, Physical properties of the coexisting phases and thermochemical
   * properties of the H2O component in boiling NaCl solutions, Geological Survey
   * Bulletin, 1421-A (1976).
   */
  CPPUNIT_TEST(vapor);

  /**
   * Verify calculation of halite solubility using data from
   * Bodnar et al, Synthetic fluid inclusions in natural quartz, III.
   * Determination of phase equilibrium properties in the system H2O-NaCl
   * to 1000C and 1500 bars, Geocehmica et Cosmochemica Acta, 49, 1861-1873 (1985).
   * Note that the average of the range quoted has been used for each point.
   */
  CPPUNIT_TEST(solubility);

  /**
   * Verify calculation of brine properties.
   * Experimental density values from Pitzer et al, Thermodynamic properties
   * of aqueous sodium chloride solution, Journal of Physical and Chemical
   * Reference Data, 13, 1-102 (1984)
   *
   * Experimental viscosity values from Phillips et al, Viscosity of NaCl and
   * other solutions up to 350C and 50MPa pressures, LBL-11586 (1980)
   *
   * Thermal conductivity values from Ozbek and Phillips, Thermal conductivity of
   * aqueous NaCl solutions from 20C to 330C, LBL-9086 (1980)
   *
   *  It is difficult to compare enthalpy and cp with experimental data, so
   * instead we recreate the data presented in Figures 11 and 12 of
   * Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar,
   * and 0 to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007)
   */
  CPPUNIT_TEST(properties);

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

  void vapor();
  void solubility();
  void properties();
  void derivatives();

private:
  MooseApp * _app;
  Factory * _factory;
  MooseMesh * _mesh;
  FEProblem * _fe_problem;
  const BrineFluidProperties * _fp;
  const SinglePhaseFluidPropertiesPT * _water_fp;
};

#endif // BRINEFLUIDPROPERTIESTEST_H
