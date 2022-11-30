//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Water97FluidPropertiesTest.h"
#include "SinglePhaseFluidPropertiesTestUtils.h"

/**
 * Test that the fluid name is correctly returned
 */
TEST_F(Water97FluidPropertiesTest, fluidName) { EXPECT_EQ(_fp->fluidName(), "water"); }

/**
 * Test that the molar mass is correctly returned
 */
TEST_F(Water97FluidPropertiesTest, molarMass) { ABS_TEST(_fp->molarMass(), 18.015e-3, 1.0e-15); }

/**
 * Test that the critical properties are correctly returned
 */
TEST_F(Water97FluidPropertiesTest, criticalProperties)
{
  ABS_TEST(_fp->criticalPressure(), 22.064e6, 1.0e-15);
  ABS_TEST(_fp->criticalTemperature(), 647.096, 1.0e-15);
  ABS_TEST(_fp->criticalDensity(), 322.0, 1.0e-15);
}

/**
 * Test that the triple point properties are correctly returned
 */
TEST_F(Water97FluidPropertiesTest, triplePointProperties)
{
  ABS_TEST(_fp->triplePointPressure(), 611.657, 1.0e-15);
  ABS_TEST(_fp->triplePointTemperature(), 273.16, 1.0e-15);
}

/**
 * Verify that the correct region is provided for a given pressure and
 * temperature. Also verify that an error is thrown if pressure and temperature
 * are outside the range of validity
 */
TEST_F(Water97FluidPropertiesTest, inRegion)
{
  // Region 1
  EXPECT_EQ(_fp->inRegion(3.0e6, 300), (unsigned int)1);
  EXPECT_EQ(_fp->inRegion(80.0e6, 300), (unsigned int)1);
  EXPECT_EQ(_fp->inRegion(3.0e6, 500), (unsigned int)1);

  // Region 2
  EXPECT_EQ(_fp->inRegion(3.5e3, 300), (unsigned int)2);
  EXPECT_EQ(_fp->inRegion(30.0e6, 700), (unsigned int)2);
  EXPECT_EQ(_fp->inRegion(30.0e6, 700), (unsigned int)2);

  // Region 3
  EXPECT_EQ(_fp->inRegion(25.588e6, 650), (unsigned int)3);
  EXPECT_EQ(_fp->inRegion(22.298e6, 650), (unsigned int)3);
  EXPECT_EQ(_fp->inRegion(78.32e6, 750), (unsigned int)3);

  // Region 5
  EXPECT_EQ(_fp->inRegion(0.5e6, 1500), (unsigned int)5);
  EXPECT_EQ(_fp->inRegion(30.0e6, 1500), (unsigned int)5);
  EXPECT_EQ(_fp->inRegion(30.0e6, 2000), (unsigned int)5);

  // Test out of range errors
  try
  {
    // Trigger invalid pressure error
    _fp->inRegion(101.0e6, 300.0);
    // TODO: this test fails with the following line that should be uncommented:
    // FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Pressure 1.01e+08 is out of range in fp: inRegion()"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    // Trigger another invalid pressure error
    _fp->inRegion(51.0e6, 1200.0);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Pressure 5.1e+07 is out of range in fp: inRegion()"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    // Trigger invalid temperature error
    _fp->inRegion(5.0e6, 2001.0);
    // TODO: this test fails with the following line that should be uncommented:
    // FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Temperature 2001 is out of range in fp: inRegion()"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

/**
 * Verify calculation of the boundary between regions 2 and 3
 * using the verification point (P,T) = (16.5291643 MPa, 623.15 K)
 * Revised Release on the IAPWS Industrial Formulation 1997 for the
 * Thermodynamic Properties of Water and Steam, IAPWS 2007
 */
TEST_F(Water97FluidPropertiesTest, b23)
{
  REL_TEST(_fp->b23T(16.5291643e6), 623.15, 1.0e-8);
  REL_TEST(_fp->b23p(623.15), 16.5291643e6, 1.0e-8);
}

/**
 * Verify calculation of the boundary between regions 2b and 2c for the
 * backwards equation T(p,h) using the verification point
 * (p,h) = (100 MPa, 0.3516004323e4 kj/kg) from
 * Revised Release on the IAPWS Industrial Formulation 1997 for the
 * Thermodynamic Properties of Water and Steam, IAPWS 2007
 */
TEST_F(Water97FluidPropertiesTest, b2bc) { REL_TEST(_fp->b2bc(100.0e6), 0.3516004323e7, 1.0e-8); }

/**
 * Verify calculation of the boundary between regions 3a and 3b for the
 * backwards equation T(p,h) using the verification point
 * (p,h) = (25 MPa, 2.095936454e3 kj/kg) from
 * Revised Supplementary Release on Backward Equations for
 * the Functions T(p,h), v(p,h) and T(p,s), v(p,s) for Region 3 of the IAPWS
 * Industrial Formulation 1997 for the Thermodynamic Properties of Water and
 * Steam
 */
TEST_F(Water97FluidPropertiesTest, b3ab) { REL_TEST(_fp->b3ab(25.0e6), 2.095936454e6, 1.0e-8); }

/**
 * Verify calculation of water properties in region 4 (saturation line)
 * using the verification values given in Table 35 of
 * Revised Release on the IAPWS Industrial Formulation 1997 for the
 * Thermodynamic Properties of Water and Steam, IAPWS 2007
 */
TEST_F(Water97FluidPropertiesTest, vaporPressure)
{
  REL_TEST(_fp->vaporPressure(300), 3.53658941e3, 1.0e-8);
  REL_TEST(_fp->vaporPressure(500), 2.63889776e6, 1.0e-8);
  REL_TEST(_fp->vaporPressure(600), 12.3443146e6, 1.0e-8);
}

/**
 * Verify calculation of water properties in region 4 (saturation line)
 * using the verification values given in Table 36 of
 * Revised Release on the IAPWS Industrial Formulation 1997 for the
 * Thermodynamic Properties of Water and Steam, IAPWS 2007
 */
TEST_F(Water97FluidPropertiesTest, vaporTemperature)
{
  REL_TEST(_fp->vaporTemperature(0.1e6), 372.755919, 1.0e-8);
  REL_TEST(_fp->vaporTemperature(1.0e6), 453.035632, 1.0e-8);
  REL_TEST(_fp->vaporTemperature(10.0e6), 584.149488, 1.0e-8);
}

/**
 * Verify calculation of the subregion in all 26 subregions in region 3 from
 * Revised Supplementary Release on Backward Equations for
 * Specific Volume as a Function of Pressure and Temperature v(p,T)
 * for Region 3 of the IAPWS Industrial Formulation 1997 for the
 * Thermodynamic Properties of Water and Steam
 */
TEST_F(Water97FluidPropertiesTest, subregion3)
{
  EXPECT_EQ(_fp->subregion3(50.0e6, 630.0), (unsigned int)0);
  EXPECT_EQ(_fp->subregion3(80.0e6, 670.0), (unsigned int)0);
  EXPECT_EQ(_fp->subregion3(50.0e6, 710.0), (unsigned int)1);
  EXPECT_EQ(_fp->subregion3(80.0e6, 750.0), (unsigned int)1);
  EXPECT_EQ(_fp->subregion3(20.0e6, 630.0), (unsigned int)2);
  EXPECT_EQ(_fp->subregion3(30.0e6, 650.0), (unsigned int)2);
  EXPECT_EQ(_fp->subregion3(26.0e6, 656.0), (unsigned int)3);
  EXPECT_EQ(_fp->subregion3(30.0e6, 670.0), (unsigned int)3);
  EXPECT_EQ(_fp->subregion3(26.0e6, 661.0), (unsigned int)4);
  EXPECT_EQ(_fp->subregion3(30.0e6, 675.0), (unsigned int)4);
  EXPECT_EQ(_fp->subregion3(26.0e6, 671.0), (unsigned int)5);
  EXPECT_EQ(_fp->subregion3(30.0e6, 690.0), (unsigned int)5);
  EXPECT_EQ(_fp->subregion3(23.6e6, 649.0), (unsigned int)6);
  EXPECT_EQ(_fp->subregion3(24.0e6, 650.0), (unsigned int)6);
  EXPECT_EQ(_fp->subregion3(23.6e6, 652.0), (unsigned int)7);
  EXPECT_EQ(_fp->subregion3(24.0e6, 654.0), (unsigned int)7);
  EXPECT_EQ(_fp->subregion3(23.6e6, 653.0), (unsigned int)8);
  EXPECT_EQ(_fp->subregion3(24.0e6, 655.0), (unsigned int)8);
  EXPECT_EQ(_fp->subregion3(23.5e6, 655.0), (unsigned int)9);
  EXPECT_EQ(_fp->subregion3(24.0e6, 660.0), (unsigned int)9);
  EXPECT_EQ(_fp->subregion3(23.0e6, 660.0), (unsigned int)10);
  EXPECT_EQ(_fp->subregion3(24.0e6, 670.0), (unsigned int)10);
  EXPECT_EQ(_fp->subregion3(22.6e6, 646.0), (unsigned int)11);
  EXPECT_EQ(_fp->subregion3(23.0e6, 646.0), (unsigned int)11);
  EXPECT_EQ(_fp->subregion3(22.6e6, 648.6), (unsigned int)12);
  EXPECT_EQ(_fp->subregion3(22.8e6, 649.3), (unsigned int)12);
  EXPECT_EQ(_fp->subregion3(22.6e6, 649.0), (unsigned int)13);
  EXPECT_EQ(_fp->subregion3(22.8e6, 649.7), (unsigned int)13);
  EXPECT_EQ(_fp->subregion3(22.6e6, 649.1), (unsigned int)14);
  EXPECT_EQ(_fp->subregion3(22.8e6, 649.9), (unsigned int)14);
  EXPECT_EQ(_fp->subregion3(22.6e6, 649.4), (unsigned int)15);
  EXPECT_EQ(_fp->subregion3(22.8e6, 650.2), (unsigned int)15);
  EXPECT_EQ(_fp->subregion3(21.1e6, 640.0), (unsigned int)16);
  EXPECT_EQ(_fp->subregion3(21.8e6, 643.0), (unsigned int)16);
  EXPECT_EQ(_fp->subregion3(21.1e6, 644.0), (unsigned int)17);
  EXPECT_EQ(_fp->subregion3(21.8e6, 648.0), (unsigned int)17);
  EXPECT_EQ(_fp->subregion3(19.1e6, 635.0), (unsigned int)18);
  EXPECT_EQ(_fp->subregion3(20.0e6, 638.0), (unsigned int)18);
  EXPECT_EQ(_fp->subregion3(17.0e6, 626.0), (unsigned int)19);
  EXPECT_EQ(_fp->subregion3(20.0e6, 640.0), (unsigned int)19);
  EXPECT_EQ(_fp->subregion3(21.5e6, 644.6), (unsigned int)20);
  EXPECT_EQ(_fp->subregion3(22.0e6, 646.1), (unsigned int)20);
  EXPECT_EQ(_fp->subregion3(22.5e6, 648.6), (unsigned int)21);
  EXPECT_EQ(_fp->subregion3(22.3e6, 647.9), (unsigned int)21);
  EXPECT_EQ(_fp->subregion3(22.15e6, 647.5), (unsigned int)22);
  EXPECT_EQ(_fp->subregion3(22.3e6, 648.1), (unsigned int)22);
  EXPECT_EQ(_fp->subregion3(22.11e6, 648.0), (unsigned int)23);
  EXPECT_EQ(_fp->subregion3(22.3e6, 649.0), (unsigned int)23);
  EXPECT_EQ(_fp->subregion3(22.0e6, 646.84), (unsigned int)24);
  EXPECT_EQ(_fp->subregion3(22.064e6, 647.05), (unsigned int)24);
  EXPECT_EQ(_fp->subregion3(22.0e6, 646.89), (unsigned int)25);
  EXPECT_EQ(_fp->subregion3(22.064e6, 647.15), (unsigned int)25);
}

/**
 * Verify calculation of the density in all 26 subregions in region 3 from
 * Revised Supplementary Release on Backward Equations for
 * Specific Volume as a Function of Pressure and Temperature v(p,T)
 * for Region 3 of the IAPWS Industrial Formulation 1997 for the
 * Thermodynamic Properties of Water and Steam
 */
TEST_F(Water97FluidPropertiesTest, subregion3Density)
{
  const Real tol = 1.0e-8;

  REL_TEST(_fp->densityRegion3(50.0e6, 630.0), 1.0 / 0.001470853100, tol);
  REL_TEST(_fp->densityRegion3(80.0e6, 670.0), 1.0 / 0.001503831359, tol);
  REL_TEST(_fp->densityRegion3(50.0e6, 710.0), 1.0 / 0.002204728587, tol);
  REL_TEST(_fp->densityRegion3(80.0e6, 750.0), 1.0 / 0.001973692940, tol);
  REL_TEST(_fp->densityRegion3(20.0e6, 630.0), 1.0 / 0.001761696406, tol);
  REL_TEST(_fp->densityRegion3(30.0e6, 650.0), 1.0 / 0.001819560617, tol);
  REL_TEST(_fp->densityRegion3(26.0e6, 656.0), 1.0 / 0.002245587720, tol);
  REL_TEST(_fp->densityRegion3(30.0e6, 670.0), 1.0 / 0.002506897702, tol);
  REL_TEST(_fp->densityRegion3(26.0e6, 661.0), 1.0 / 0.002970225962, tol);
  REL_TEST(_fp->densityRegion3(30.0e6, 675.0), 1.0 / 0.003004627086, tol);
  REL_TEST(_fp->densityRegion3(26.0e6, 671.0), 1.0 / 0.005019029401, tol);
  REL_TEST(_fp->densityRegion3(30.0e6, 690.0), 1.0 / 0.004656470142, tol);
  REL_TEST(_fp->densityRegion3(23.6e6, 649.0), 1.0 / 0.002163198378, tol);
  REL_TEST(_fp->densityRegion3(24.0e6, 650.0), 1.0 / 0.002166044161, tol);
  REL_TEST(_fp->densityRegion3(23.6e6, 652.0), 1.0 / 0.002651081407, tol);
  REL_TEST(_fp->densityRegion3(24.0e6, 654.0), 1.0 / 0.002967802335, tol);
  REL_TEST(_fp->densityRegion3(23.6e6, 653.0), 1.0 / 0.003273916816, tol);
  REL_TEST(_fp->densityRegion3(24.0e6, 655.0), 1.0 / 0.003550329864, tol);
  REL_TEST(_fp->densityRegion3(23.5e6, 655.0), 1.0 / 0.004545001142, tol);
  REL_TEST(_fp->densityRegion3(24.0e6, 660.0), 1.0 / 0.005100267704, tol);
  REL_TEST(_fp->densityRegion3(23.0e6, 660.0), 1.0 / 0.006109525997, tol);
  REL_TEST(_fp->densityRegion3(24.0e6, 670.0), 1.0 / 0.006427325645, tol);
  REL_TEST(_fp->densityRegion3(22.6e6, 646.0), 1.0 / 0.002117860851, tol);
  REL_TEST(_fp->densityRegion3(23.0e6, 646.0), 1.0 / 0.002062374674, tol);
  REL_TEST(_fp->densityRegion3(22.6e6, 648.6), 1.0 / 0.002533063780, tol);
  REL_TEST(_fp->densityRegion3(22.8e6, 649.3), 1.0 / 0.002572971781, tol);
  REL_TEST(_fp->densityRegion3(22.6e6, 649.0), 1.0 / 0.002923432711, tol);
  REL_TEST(_fp->densityRegion3(22.8e6, 649.7), 1.0 / 0.002913311494, tol);
  REL_TEST(_fp->densityRegion3(22.6e6, 649.1), 1.0 / 0.003131208996, tol);
  REL_TEST(_fp->densityRegion3(22.8e6, 649.9), 1.0 / 0.003221160278, tol);
  REL_TEST(_fp->densityRegion3(22.6e6, 649.4), 1.0 / 0.003715596186, tol);
  REL_TEST(_fp->densityRegion3(22.8e6, 650.2), 1.0 / 0.003664754790, tol);
  REL_TEST(_fp->densityRegion3(21.1e6, 640.0), 1.0 / 0.001970999272, tol);
  REL_TEST(_fp->densityRegion3(21.8e6, 643.0), 1.0 / 0.002043919161, tol);
  REL_TEST(_fp->densityRegion3(21.1e6, 644.0), 1.0 / 0.005251009921, tol);
  REL_TEST(_fp->densityRegion3(21.8e6, 648.0), 1.0 / 0.005256844741, tol);
  REL_TEST(_fp->densityRegion3(19.1e6, 635.0), 1.0 / 0.001932829079, tol);
  REL_TEST(_fp->densityRegion3(20.0e6, 638.0), 1.0 / 0.001985387227, tol);
  REL_TEST(_fp->densityRegion3(17.0e6, 626.0), 1.0 / 0.008483262001, tol);
  REL_TEST(_fp->densityRegion3(20.0e6, 640.0), 1.0 / 0.006227528101, tol);
  REL_TEST(_fp->densityRegion3(21.5e6, 644.6), 1.0 / 0.002268366647, tol);
  REL_TEST(_fp->densityRegion3(22.0e6, 646.1), 1.0 / 0.002296350553, tol);
  REL_TEST(_fp->densityRegion3(22.5e6, 648.6), 1.0 / 0.002832373260, tol);
  REL_TEST(_fp->densityRegion3(22.3e6, 647.9), 1.0 / 0.002811424405, tol);
  REL_TEST(_fp->densityRegion3(22.15e6, 647.5), 1.0 / 0.003694032281, tol);
  REL_TEST(_fp->densityRegion3(22.3e6, 648.1), 1.0 / 0.003622226305, tol);
  REL_TEST(_fp->densityRegion3(22.11e6, 648.0), 1.0 / 0.004528072649, tol);
  REL_TEST(_fp->densityRegion3(22.3e6, 649.0), 1.0 / 0.004556905799, tol);
  REL_TEST(_fp->densityRegion3(22.0e6, 646.84), 1.0 / 0.002698354719, tol);
  REL_TEST(_fp->densityRegion3(22.064e6, 647.05), 1.0 / 0.002717655648, tol);
  REL_TEST(_fp->densityRegion3(22.0e6, 646.89), 1.0 / 0.003798732962, tol);
  REL_TEST(_fp->densityRegion3(22.064e6, 647.15), 1.0 / 0.003701940010, tol);
}

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
TEST_F(Water97FluidPropertiesTest, properties)
{
  Real p0, p1, p2, T0, T1, T2;

  const Real tol = 1.0e-8;
  const Real tol2 = 1.0e-12;

  // Region 1 properties
  p0 = 3.0e6;
  p1 = 80.0e6;
  p2 = 3.0e6;
  T0 = 300.0;
  T1 = 300.0;
  T2 = 500.0;

  REL_TEST(_fp->rho_from_p_T(p0, T0), 1.0 / 0.00100215168, tol);
  REL_TEST(_fp->rho_from_p_T(p1, T1), 1.0 / 0.000971180894, tol);
  REL_TEST(_fp->rho_from_p_T(p2, T2), 1.0 / 0.00120241800, tol);
  REL_TEST(_fp->h_from_p_T(p0, T0), 115.331273e3, tol);
  REL_TEST(_fp->h_from_p_T(p1, T1), 184.142828e3, tol);
  REL_TEST(_fp->h_from_p_T(p2, T2), 975.542239e3, tol);
  REL_TEST(_fp->e_from_p_T(p0, T0), 112.324818e3, tol);
  REL_TEST(_fp->e_from_p_T(p1, T1), 106.448356e3, tol);
  REL_TEST(_fp->e_from_p_T(p2, T2), 971.934985e3, tol);
  REL_TEST(_fp->s_from_p_T(p0, T0), 0.392294792e3, tol);
  REL_TEST(_fp->s_from_p_T(p1, T1), 0.368563852e3, tol);
  REL_TEST(_fp->s_from_p_T(p2, T2), 2.58041912e3, tol);
  REL_TEST(_fp->cp_from_p_T(p0, T0), 4.17301218e3, tol);
  REL_TEST(_fp->cp_from_p_T(p1, T1), 4.01008987e3, tol);
  REL_TEST(_fp->cp_from_p_T(p2, T2), 4.65580682e3, tol);
  REL_TEST(_fp->c_from_p_T(p0, T0), 1507.73921, tol);
  REL_TEST(_fp->c_from_p_T(p1, T1), 1634.69054, tol);
  REL_TEST(_fp->c_from_p_T(p2, T2), 1240.71337, tol);

  // Lower tolerance for cv as it is compared with values from NIST
  REL_TEST(_fp->cv_from_p_T(p0, T0), 4.1207e3, REL_TOL_EXTERNAL_VALUE);

  // Region 2 properties
  p0 = 3.5e3;
  p1 = 3.5e3;
  p2 = 30.0e6;
  T0 = 300.0;
  T1 = 700.0;
  T2 = 700.0;

  REL_TEST(_fp->rho_from_p_T(p0, T0), 1.0 / 39.4913866, tol);
  REL_TEST(_fp->rho_from_p_T(p1, T1), 1.0 / 92.3015898, tol);
  REL_TEST(_fp->rho_from_p_T(p2, T2), 1.0 / 0.00542946619, tol);
  REL_TEST(_fp->h_from_p_T(p0, T0), 2549.91145e3, tol);
  REL_TEST(_fp->h_from_p_T(p1, T1), 3335.68375e3, tol);
  REL_TEST(_fp->h_from_p_T(p2, T2), 2631.49474e3, tol);
  REL_TEST(_fp->e_from_p_T(p0, T0), 2411.6916e3, tol);
  REL_TEST(_fp->e_from_p_T(p1, T1), 3012.62819e3, tol);
  REL_TEST(_fp->e_from_p_T(p2, T2), 2468.61076e3, tol);
  REL_TEST(_fp->s_from_p_T(p0, T0), 8.52238967e3, tol);
  REL_TEST(_fp->s_from_p_T(p1, T1), 10.1749996e3, tol);
  REL_TEST(_fp->s_from_p_T(p2, T2), 5.17540298e3, tol);
  REL_TEST(_fp->cp_from_p_T(p0, T0), 1.91300162e3, tol);
  REL_TEST(_fp->cp_from_p_T(p1, T1), 2.08141274e3, tol);
  REL_TEST(_fp->cp_from_p_T(p2, T2), 10.3505092e3, tol);
  REL_TEST(_fp->c_from_p_T(p0, T0), 427.920172, tol);
  REL_TEST(_fp->c_from_p_T(p1, T1), 644.289068, tol);
  REL_TEST(_fp->c_from_p_T(p2, T2), 480.386523, tol);

  // Lower tolerance for cv as it is compared with values from NIST
  REL_TEST(_fp->cv_from_p_T(p0, T0), 1.4415e3, REL_TOL_EXTERNAL_VALUE);

  // Region 3 properties
  p0 = 25.5837018e6;
  p1 = 22.2930643e6;
  p2 = 78.3095639e6;
  T0 = 650.0;
  T1 = 650.0;
  T2 = 750.0;

  // Note: lower tolerance in this region as density is calculated using backwards equation
  REL_TEST(_fp->rho_from_p_T(p0, T0), 500.0, 1.0e-5);
  REL_TEST(_fp->rho_from_p_T(p1, T1), 200.0, 1.0e-5);
  REL_TEST(_fp->rho_from_p_T(p2, T2), 500.0, 1.0e-5);
  REL_TEST(_fp->h_from_p_T(p0, T0), 1863.43019e3, 1.0e-5);
  REL_TEST(_fp->h_from_p_T(p1, T1), 2375.12401e3, 1.0e-5);
  REL_TEST(_fp->h_from_p_T(p2, T2), 2258.68845e3, 1.0e-5);
  REL_TEST(_fp->e_from_p_T(p0, T0), 1812.26279e3, 1.0e-5);
  REL_TEST(_fp->e_from_p_T(p1, T1), 2263.65868e3, 1.0e-5);
  REL_TEST(_fp->e_from_p_T(p2, T2), 2102.06932e3, 1.0e-5);
  REL_TEST(_fp->s_from_p_T(p0, T0), 4.05427273e3, 1.0e-5);
  REL_TEST(_fp->s_from_p_T(p1, T1), 4.85438792e3, 1.0e-5);
  REL_TEST(_fp->s_from_p_T(p2, T2), 4.46971906e3, 1.0e-5);
  REL_TEST(_fp->cp_from_p_T(p0, T0), 13.8935717e3, 1.0e-4);
  REL_TEST(_fp->cp_from_p_T(p1, T1), 44.6579342e3, 1.0e-5);
  REL_TEST(_fp->cp_from_p_T(p2, T2), 6.34165359e3, 1.0e-5);
  REL_TEST(_fp->c_from_p_T(p0, T0), 502.005554, 1.0e-5);
  REL_TEST(_fp->c_from_p_T(p1, T1), 383.444594, 1.0e-5);
  REL_TEST(_fp->c_from_p_T(p2, T2), 760.696041, 1.0e-5);

  // Lower tolerance for cv as it is compared with values from NIST
  REL_TEST(_fp->cv_from_p_T(p0, T0), 3.1910e3, REL_TOL_EXTERNAL_VALUE);

  // Region 5 properties
  p0 = 0.5e6;
  p1 = 30.0e6;
  p2 = 30.0e6;
  T0 = 1500.0;
  T1 = 1500.0;
  T2 = 2000.0;

  REL_TEST(_fp->rho_from_p_T(p0, T0), 1.0 / 1.38455090, tol);
  REL_TEST(_fp->rho_from_p_T(p1, T1), 1.0 / 0.0230761299, tol);
  REL_TEST(_fp->rho_from_p_T(p2, T2), 1.0 / 0.0311385219, tol);
  REL_TEST(_fp->h_from_p_T(p0, T0), 5219.76855e3, tol);
  REL_TEST(_fp->h_from_p_T(p1, T1), 5167.23514e3, tol);
  REL_TEST(_fp->h_from_p_T(p2, T2), 6571.22604e3, tol);
  REL_TEST(_fp->e_from_p_T(p0, T0), 4527.4931e3, tol);
  REL_TEST(_fp->e_from_p_T(p1, T1), 4474.95124e3, tol);
  REL_TEST(_fp->e_from_p_T(p2, T2), 5637.07038e3, tol);
  REL_TEST(_fp->s_from_p_T(p0, T0), 9.65408875e3, tol);
  REL_TEST(_fp->s_from_p_T(p1, T1), 7.72970133e3, tol);
  REL_TEST(_fp->s_from_p_T(p2, T2), 8.53640523e3, tol);
  REL_TEST(_fp->cp_from_p_T(p0, T0), 2.61609445e3, tol);
  REL_TEST(_fp->cp_from_p_T(p1, T1), 2.72724317e3, tol);
  REL_TEST(_fp->cp_from_p_T(p2, T2), 2.88569882e3, tol);
  REL_TEST(_fp->c_from_p_T(p0, T0), 917.06869, tol);
  REL_TEST(_fp->c_from_p_T(p1, T1), 928.548002, tol);
  REL_TEST(_fp->c_from_p_T(p2, T2), 1067.36948, tol);

  // Lower tolerance for cv as it is compared with values from NIST
  REL_TEST(_fp->cv_from_p_T(p0, T0), 2.1534e3, REL_TOL_EXTERNAL_VALUE);

  // Viscosity
  ABS_TEST(_fp->mu_from_rho_T(998.0, 298.15), 889.735100e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(1200.0, 298.15), 1437.649467e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(1000.0, 373.15), 307.883622e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(1.0, 433.15), 14.538324e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(1000.0, 433.15), 217.685358e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(1.0, 873.15), 32.619287e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(100.0, 873.15), 35.802262e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(600.0, 873.15), 77.430195e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(1.0, 1173.15), 44.217245e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(100.0, 1173.15), 47.640433e-6, tol2);
  ABS_TEST(_fp->mu_from_rho_T(400.0, 1173.15), 64.154608e-6, tol2);
  REL_TEST(_fp->mu_from_p_T(1e6, 298.15), 889.898581797e-6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->mu_from_p_T(2e6, 298.15), 889.763899645e-6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->mu_from_p_T(1e6, 373.15), 281.825180491e-6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->mu_from_p_T(2e6, 373.15), 282.09550632e-6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->mu_from_p_T(1e6, 433.15), 170.526801634e-6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->mu_from_p_T(2e6, 433.15), 170.780193827e-6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->mu_from_p_T(1e6, 873.15), 3.2641885983e-5, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->mu_from_p_T(2e6, 873.15), 3.26820969808e-5, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->mu_from_p_T(1e6, 1173.15), 4.42374919686e-5, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->mu_from_p_T(2e6, 1173.15), 4.42823959629e-5, REL_TOL_EXTERNAL_VALUE);

  // Thermal conductivity
  REL_TEST(_fp->k_from_p_T(1.0e6, 323.15), 0.641, 1.0e-4);
  REL_TEST(_fp->k_from_p_T(20.0e6, 623.15), 0.4541, 1.0e-4);
  REL_TEST(_fp->k_from_p_T(50.0e6, 773.15), 0.2055, 1.0e-4);

  ABS_TEST(_fp->k_from_p_T(1.0e6, 323.15), 0.640972, 5e-7);
  ABS_TEST(_fp->k_from_p_T(20.0e6, 623.15), 0.454131, 7e-7);
  ABS_TEST(_fp->k_from_p_T(50.0e6, 773.15), 0.205485, 5e-7);

  // Backwards equation T(p,h)
  // Region 1
  REL_TEST(_fp->T_from_p_h(3.0e6, 500.0e3), 0.391798509e3, tol);
  REL_TEST(_fp->T_from_p_h(80.0e6, 500.0e3), 0.378108626e3, tol);
  REL_TEST(_fp->T_from_p_h(80.0e6, 1500.0e3), 0.611041229e3, tol);

  // Region 2 (subregion a)
  REL_TEST(_fp->T_from_p_h(1.0e3, 3000.0e3), 0.534433241e3, tol);
  REL_TEST(_fp->T_from_p_h(3.0e6, 3000.0e3), 0.575373370e3, tol);
  REL_TEST(_fp->T_from_p_h(3.0e6, 4000.0e3), 0.101077577e4, tol);

  // Region 2 (subregion b)
  REL_TEST(_fp->T_from_p_h(5.0e6, 3500.0e3), 0.801299102e3, tol);
  REL_TEST(_fp->T_from_p_h(5.0e6, 4000.0e3), 0.101531583e4, tol);
  REL_TEST(_fp->T_from_p_h(25.0e6, 3500.0e3), 0.875279054e3, tol);

  // Region 2 (subregion c)
  REL_TEST(_fp->T_from_p_h(40.0e6, 2700.0e3), 0.743056411e3, tol);
  REL_TEST(_fp->T_from_p_h(60.0e6, 2700.0e3), 0.791137067e3, tol);
  REL_TEST(_fp->T_from_p_h(60.0e6, 3200.0e3), 0.882756860e3, tol);

  // Region 3 (subregion a)
  REL_TEST(_fp->T_from_p_h(20.0e6, 1700.0e3), 0.6293083892e3, tol);
  REL_TEST(_fp->T_from_p_h(50.0e6, 2000.0e3), 0.6905718338e3, tol);
  REL_TEST(_fp->T_from_p_h(100.0e6, 2100.0e3), 0.7336163014e3, tol);

  // Region 3 (subregion b)
  REL_TEST(_fp->T_from_p_h(20.0e6, 2500.0e3), 0.6418418053e3, tol);
  REL_TEST(_fp->T_from_p_h(50.0e6, 2400.0e3), 0.7351848618e3, tol);
  REL_TEST(_fp->T_from_p_h(100.0e6, 2700.0e3), 0.8420460876e3, tol);
}

/**
 * Verify calculation of the derivatives in all regions by comparing with finite
 * differences
 */
TEST_F(Water97FluidPropertiesTest, derivatives)
{
  const Real tol = REL_TOL_DERIVATIVE;

  // Region 1
  Real p = 3.0e6;
  Real T = 300.0;
  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);

  // Region 2
  p = 3.5e3;
  T = 300.0;
  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);

  // Region 3
  p = 26.0e6;
  T = 650.0;
  DERIV_TEST(_fp->rho_from_p_T, p, T, 1.0e-2);
  DERIV_TEST(_fp->e_from_p_T, p, T, 1.0e-2);
  DERIV_TEST(_fp->h_from_p_T, p, T, 1.0e-2);

  // Region 4 (saturation curve)
  T = 300.0;
  const Real dT = 1.0e-4;

  Real dpSat_dT_fd = (_fp->vaporPressure(T + dT) - _fp->vaporPressure(T - dT)) / (2.0 * dT);
  Real pSat = 0.0, dpSat_dT = 0.0;
  _fp->vaporPressure(T, pSat, dpSat_dT);

  REL_TEST(dpSat_dT, dpSat_dT_fd, 1.0e-6);

  // Region 5
  p = 30.0e6;
  T = 1500.0;
  DERIV_TEST(_fp->rho_from_p_T, p, T, tol);
  DERIV_TEST(_fp->e_from_p_T, p, T, tol);
  DERIV_TEST(_fp->h_from_p_T, p, T, tol);

  // Viscosity
  Real rho = 998.0, drho_dp = 0.0, drho_dT = 0.0;
  T = 298.15;
  Real drho = 1.0e-4;

  Real dmu_drho_fd =
      (_fp->mu_from_rho_T(rho + drho, T) - _fp->mu_from_rho_T(rho - drho, T)) / (2.0 * drho);
  Real mu = 0.0, dmu_drho = 0.0, dmu_dT = 0.0;
  _fp->mu_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);

  ABS_TEST(mu, _fp->mu_from_rho_T(rho, T), 1.0e-15);
  REL_TEST(dmu_drho, dmu_drho_fd, 1.0e-6);

  // To properly test derivative wrt temperature, use p and T and calculate density,
  // so that the change in density wrt temperature is included
  p = 1.0e6;
  _fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  _fp->mu_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);
  Real dmu_dT_fd = (_fp->mu_from_rho_T(_fp->rho_from_p_T(p, T + dT), T + dT) -
                    _fp->mu_from_rho_T(_fp->rho_from_p_T(p, T - dT), T - dT)) /
                   (2.0 * dT);

  REL_TEST(dmu_dT, dmu_dT_fd, 1.0e-6);

  // Check derivative of viscosity wrt pressure
  Real dp = 1.0e1;

  Real dmu_dp_fd = (_fp->mu_from_p_T(p + dp, T) - _fp->mu_from_p_T(p - dp, T)) / (2.0 * dp);
  Real dmu_dp = 0.0;
  _fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);

  REL_TEST(dmu_dp, dmu_dp_fd, 1.0e-5);

  // Check derivatives of temperature calculated using pressure and enthalpy using AD
  DualReal adp = 3.0e6;
  Moose::derivInsert(adp.derivatives(), 0, 1.0);

  DualReal adh = 4.0e6;
  Moose::derivInsert(adh.derivatives(), 1, 1.0);

  DualReal adT = _ad_fp->T_from_p_h(adp, adh);

  REL_TEST(adT.value(), 0.101077577e4, 1.0e-8);

  Real dT_dp_fd = (_fp->T_from_p_h(adp.value() + dp, adh.value()) -
                   _fp->T_from_p_h(adp.value() - dp, adh.value())) /
                  (2.0 * dp);

  REL_TEST(adT.derivatives()[0], dT_dp_fd, tol);

  const Real dh = 1.0;
  Real dT_dh_fd = (_fp->T_from_p_h(adp.value(), adh.value() + dh) -
                   _fp->T_from_p_h(adp.value(), adh.value() - dh)) /
                  (2.0 * dh);

  REL_TEST(adT.derivatives()[1], dT_dh_fd, tol);
}

/**
 * Verify that the methods that return multiple properties in one call return identical
 * values as the individual methods
 */
TEST_F(Water97FluidPropertiesTest, combined)
{
  const Real p = 1.0e6;
  const Real T = 300.0;
  const Real tol = REL_TOL_CONSISTENCY;

  // Single property methods
  Real rho, drho_dp, drho_dT;
  _fp->rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real mu, dmu_dp, dmu_dT;
  _fp->mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  Real e, de_dp, de_dT;
  _fp->e_from_p_T(p, T, e, de_dp, de_dT);

  // Combined property methods
  Real rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT, e2, de2_dp, de2_dT;
  _fp->rho_mu_from_p_T(p, T, rho2, mu2);

  ABS_TEST(rho, rho2, tol);
  ABS_TEST(mu, mu2, tol);

  _fp->rho_mu_from_p_T(p, T, rho2, drho2_dp, drho2_dT, mu2, dmu2_dp, dmu2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(mu, mu2, tol);
  ABS_TEST(dmu_dp, dmu2_dp, tol);
  ABS_TEST(dmu_dT, dmu2_dT, tol);

  _fp->rho_e_from_p_T(p, T, rho2, drho2_dp, drho2_dT, e2, de2_dp, de2_dT);
  ABS_TEST(rho, rho2, tol);
  ABS_TEST(drho_dp, drho2_dp, tol);
  ABS_TEST(drho_dT, drho2_dT, tol);
  ABS_TEST(e, e2, tol);
  ABS_TEST(de_dp, de2_dp, tol);
  ABS_TEST(de_dT, de2_dT, tol);
}

/**
 * Verify calculation of Henry's constant using data from
 * Guidelines on the Henry's constant and vapour liquid distribution constant
 * for gases in H20 and D20 at high temperatures, IAPWS (2004).
 */
TEST_F(Water97FluidPropertiesTest, henry)
{
  const Real tol = REL_TOL_EXTERNAL_VALUE;

  // CO2 constants
  const std::vector<Real> co2{-8.55445, 4.01195, 9.52345};
  REL_TEST(_fp->henryConstant(300.0, co2), 173.63e6, tol);
  REL_TEST(_fp->henryConstant(500.0, co2), 520.79e6, tol);

  // CH4 constants
  const std::vector<Real> ch4{-10.44708, 4.66491, 12.1298};
  REL_TEST(_fp->henryConstant(400.0, ch4), 6017.1e6, REL_TOL_EXTERNAL_VALUE);
  REL_TEST(_fp->henryConstant(600.0, ch4), 801.8e6, REL_TOL_EXTERNAL_VALUE);

  // Test derivative of Henry's constant wrt temperature
  const Real dT = 1.0e-4;
  const Real dKh_dT_fd =
      (_fp->henryConstant(500.0 + dT, co2) - _fp->henryConstant(500.0 - dT, co2)) / (2.0 * dT);

  Real Kh = 0.0, dKh_dT = 0.0;
  _fp->henryConstant(500.0, co2, Kh, dKh_dT);
  REL_TEST(Kh, _fp->henryConstant(500.0, co2), REL_TOL_CONSISTENCY);
  REL_TEST(dKh_dT_fd, dKh_dT, REL_TOL_DERIVATIVE);
}

/**
 * Verify that calculations from conservative variables are accurate
 */
TEST_F(Water97FluidPropertiesTest, conservative)
{
  auto run_tests = [this](const auto & example)
  {
    typedef typename std::decay<decltype(example)>::type TestType;
    const TestType pressure = 1.01e5;
    const TestType temperature = 298.15;

    const auto rho = _fp->rho_from_p_T(pressure, temperature);
    const auto v = 1 / rho;
    auto e = _fp->e_from_p_T(pressure, temperature);

    auto [p_test, T_test] = _fp->p_T_from_v_e(v, e);
    REL_TEST(pressure, p_test, REL_TOL_CONSISTENCY);
    REL_TEST(temperature, T_test, REL_TOL_CONSISTENCY);

    decltype(p_test) rho_test;
    std::tie(rho_test, T_test) = _fp->rho_T_from_v_e(v, e);
    REL_TEST(rho, rho_test, REL_TOL_CONSISTENCY);
    REL_TEST(temperature, T_test, REL_TOL_CONSISTENCY);

    REL_TEST(_fp->k_from_p_T(pressure, temperature), _fp->k_from_v_e(v, e), REL_TOL_CONSISTENCY);
    REL_TEST(_fp->e_from_p_rho(pressure, rho), e, REL_TOL_CONSISTENCY);

    constexpr Real perturbation_factor = 1 + 1e-8;
    TestType de_dp, de_drho, de_dT;
    _fp->e_from_p_rho(pressure, rho, e, de_dp, de_drho);
    auto de_dp_diff =
        (_fp->e_from_p_rho(perturbation_factor * pressure, rho) - e) / (1e-8 * pressure);
    REL_TEST(de_dp, de_dp_diff, 1e-2);
    auto de_drho_diff = (_fp->e_from_p_rho(pressure, perturbation_factor * rho) - e) / (1e-8 * rho);
    REL_TEST(de_drho, de_drho_diff, 1e-2);

    _fp->e_from_p_T(pressure, temperature, e, de_dp, de_dT);
    de_dp_diff =
        (_fp->e_from_p_T(perturbation_factor * pressure, temperature) - e) / (1e-8 * pressure);
    REL_TEST(de_dp, de_dp_diff, 1e-2);
    auto de_dT_diff =
        (_fp->e_from_p_T(pressure, perturbation_factor * temperature) - e) / (1e-8 * temperature);
    REL_TEST(de_dT, de_dT_diff, 1e-2);

    auto h = _fp->h_from_p_T(pressure, temperature);
    std::tie(p_test, T_test) = _fp->p_T_from_v_h(v, h);
    REL_TEST(pressure, p_test, REL_TOL_CONSISTENCY);
    REL_TEST(temperature, T_test, REL_TOL_CONSISTENCY);

    REL_TEST(_fp->e_from_v_h(v, h), e, REL_TOL_CONSISTENCY);
    REL_TEST(_fp->T_from_v_e(v, e), temperature, REL_TOL_CONSISTENCY);
    REL_TEST(_fp->c_from_v_e(v, e), _fp->c_from_p_T(pressure, temperature), REL_TOL_CONSISTENCY);
    REL_TEST(_fp->cp_from_v_e(v, e), _fp->cp_from_p_T(pressure, temperature), REL_TOL_CONSISTENCY);
    REL_TEST(_fp->cv_from_v_e(v, e), _fp->cv_from_p_T(pressure, temperature), REL_TOL_CONSISTENCY);
    REL_TEST(_fp->mu_from_v_e(v, e), _fp->mu_from_p_T(pressure, temperature), REL_TOL_CONSISTENCY);
    REL_TEST(_fp->k_from_v_e(v, e), _fp->k_from_p_T(pressure, temperature), REL_TOL_CONSISTENCY);

    const Real p0 = MetaPhysicL::raw_value(pressure) * 1.01;
    const Real T0 = MetaPhysicL::raw_value(temperature) * 1.01;

    auto s = _fp->s_from_p_T(pressure, temperature);
    bool conversion_succeeded = false;
    _fp->p_T_from_h_s(h, s, p0, T0, p_test, T_test, conversion_succeeded);
    EXPECT_TRUE(conversion_succeeded);
    REL_TEST(pressure, p_test, REL_TOL_CONSISTENCY);
    REL_TEST(temperature, T_test, REL_TOL_CONSISTENCY);
  };

  run_tests(Real{});
  run_tests(ADReal{});
}
