//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Water97FluidPropertiesTest.h"

/**
 * Verify that the correct region is provided for a given pressure and
 * temperature. Also verify that an error is thrown if pressure and temperature
 * are outside the range of validity
 */
TEST_F(Water97FluidPropertiesTest, inRegion)
{
  // Region 1
  EXPECT_EQ(_fp->inRegion(3.0e6, 300), 1);
  EXPECT_EQ(_fp->inRegion(80.0e6, 300), 1);
  EXPECT_EQ(_fp->inRegion(3.0e6, 500), 1);

  // Region 2
  EXPECT_EQ(_fp->inRegion(3.5e3, 300), 2);
  EXPECT_EQ(_fp->inRegion(30.0e6, 700), 2);
  EXPECT_EQ(_fp->inRegion(30.0e6, 700), 2);

  // Region 3
  EXPECT_EQ(_fp->inRegion(25.588e6, 650), 3);
  EXPECT_EQ(_fp->inRegion(22.298e6, 650), 3);
  EXPECT_EQ(_fp->inRegion(78.32e6, 750), 3);

  // Region 5
  EXPECT_EQ(_fp->inRegion(0.5e6, 1500), 5);
  EXPECT_EQ(_fp->inRegion(30.0e6, 1500), 5);
  EXPECT_EQ(_fp->inRegion(30.0e6, 2000), 5);

  // Test out of range errors
  unsigned int region;
  try
  {
    // Trigger invalid pressure error
    region = _fp->inRegion(101.0e6, 300.0);
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
    region = _fp->inRegion(51.0e6, 1200.0);
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
    region = _fp->inRegion(5.0e6, 2001.0);
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
  REL_TEST("b23T", _fp->b23T(16.5291643e6), 623.15, 1.0e-8);
  REL_TEST("b23p", _fp->b23p(623.15), 16.5291643e6, 1.0e-8);
}

/**
 * Verify calculation of the boundary between regions 2b and 2c for the
 * backwards equation T(p,h) using the verification point
 * (p,h) = (100 MPa, 0.3516004323e4 kj/kg) from
 * Revised Release on the IAPWS Industrial Formulation 1997 for the
 * Thermodynamic Properties of Water and Steam, IAPWS 2007
 */
TEST_F(Water97FluidPropertiesTest, b2bc)
{
  REL_TEST("b2bc", _fp->b2bc(100.0e6), 0.3516004323e7, 1.0e-8);
}

/**
 * Verify calculation of the boundary between regions 3a and 3b for the
 * backwards equation T(p,h) using the verification point
 * (p,h) = (25 MPa, 2.095936454e3 kj/kg) from
 * Revised Supplementary Release on Backward Equations for
 * the Functions T(p,h), v(p,h) and T(p,s), v(p,s) for Region 3 of the IAPWS
 * Industrial Formulation 1997 for the Thermodynamic Properties of Water and
 * Steam
 */
TEST_F(Water97FluidPropertiesTest, b3ab)
{
  REL_TEST("b3ab", _fp->b3ab(25.0e6), 2.095936454e6, 1.0e-8);
}

/**
 * Verify calculation of water properties in region 4 (saturation line)
 * using the verification values given in Table 35 of
 * Revised Release on the IAPWS Industrial Formulation 1997 for the
 * Thermodynamic Properties of Water and Steam, IAPWS 2007
 */
TEST_F(Water97FluidPropertiesTest, vaporPressure)
{
  REL_TEST("vaporPressure", _fp->vaporPressure(300), 3.53658941e3, 1.0e-8);
  REL_TEST("vaporPressure", _fp->vaporPressure(500), 2.63889776e6, 1.0e-8);
  REL_TEST("vaporPressure", _fp->vaporPressure(600), 12.3443146e6, 1.0e-8);
}

/**
 * Verify calculation of water properties in region 4 (saturation line)
 * using the verification values given in Table 36 of
 * Revised Release on the IAPWS Industrial Formulation 1997 for the
 * Thermodynamic Properties of Water and Steam, IAPWS 2007
 */
TEST_F(Water97FluidPropertiesTest, vaporTemperature)
{
  REL_TEST("vaporPressure", _fp->vaporTemperature(0.1e6), 372.755919, 1.0e-8);
  REL_TEST("vaporPressure", _fp->vaporTemperature(1.0e6), 453.035632, 1.0e-8);
  REL_TEST("vaporPressure", _fp->vaporTemperature(10.0e6), 584.149488, 1.0e-8);
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
  EXPECT_EQ(_fp->subregion3(50.0e6, 630.0), 0);
  EXPECT_EQ(_fp->subregion3(80.0e6, 670.0), 0);
  EXPECT_EQ(_fp->subregion3(50.0e6, 710.0), 1);
  EXPECT_EQ(_fp->subregion3(80.0e6, 750.0), 1);
  EXPECT_EQ(_fp->subregion3(20.0e6, 630.0), 2);
  EXPECT_EQ(_fp->subregion3(30.0e6, 650.0), 2);
  EXPECT_EQ(_fp->subregion3(26.0e6, 656.0), 3);
  EXPECT_EQ(_fp->subregion3(30.0e6, 670.0), 3);
  EXPECT_EQ(_fp->subregion3(26.0e6, 661.0), 4);
  EXPECT_EQ(_fp->subregion3(30.0e6, 675.0), 4);
  EXPECT_EQ(_fp->subregion3(26.0e6, 671.0), 5);
  EXPECT_EQ(_fp->subregion3(30.0e6, 690.0), 5);
  EXPECT_EQ(_fp->subregion3(23.6e6, 649.0), 6);
  EXPECT_EQ(_fp->subregion3(24.0e6, 650.0), 6);
  EXPECT_EQ(_fp->subregion3(23.6e6, 652.0), 7);
  EXPECT_EQ(_fp->subregion3(24.0e6, 654.0), 7);
  EXPECT_EQ(_fp->subregion3(23.6e6, 653.0), 8);
  EXPECT_EQ(_fp->subregion3(24.0e6, 655.0), 8);
  EXPECT_EQ(_fp->subregion3(23.5e6, 655.0), 9);
  EXPECT_EQ(_fp->subregion3(24.0e6, 660.0), 9);
  EXPECT_EQ(_fp->subregion3(23.0e6, 660.0), 10);
  EXPECT_EQ(_fp->subregion3(24.0e6, 670.0), 10);
  EXPECT_EQ(_fp->subregion3(22.6e6, 646.0), 11);
  EXPECT_EQ(_fp->subregion3(23.0e6, 646.0), 11);
  EXPECT_EQ(_fp->subregion3(22.6e6, 648.6), 12);
  EXPECT_EQ(_fp->subregion3(22.8e6, 649.3), 12);
  EXPECT_EQ(_fp->subregion3(22.6e6, 649.0), 13);
  EXPECT_EQ(_fp->subregion3(22.8e6, 649.7), 13);
  EXPECT_EQ(_fp->subregion3(22.6e6, 649.1), 14);
  EXPECT_EQ(_fp->subregion3(22.8e6, 649.9), 14);
  EXPECT_EQ(_fp->subregion3(22.6e6, 649.4), 15);
  EXPECT_EQ(_fp->subregion3(22.8e6, 650.2), 15);
  EXPECT_EQ(_fp->subregion3(21.1e6, 640.0), 16);
  EXPECT_EQ(_fp->subregion3(21.8e6, 643.0), 16);
  EXPECT_EQ(_fp->subregion3(21.1e6, 644.0), 17);
  EXPECT_EQ(_fp->subregion3(21.8e6, 648.0), 17);
  EXPECT_EQ(_fp->subregion3(19.1e6, 635.0), 18);
  EXPECT_EQ(_fp->subregion3(20.0e6, 638.0), 18);
  EXPECT_EQ(_fp->subregion3(17.0e6, 626.0), 19);
  EXPECT_EQ(_fp->subregion3(20.0e6, 640.0), 19);
  EXPECT_EQ(_fp->subregion3(21.5e6, 644.6), 20);
  EXPECT_EQ(_fp->subregion3(22.0e6, 646.1), 20);
  EXPECT_EQ(_fp->subregion3(22.5e6, 648.6), 21);
  EXPECT_EQ(_fp->subregion3(22.3e6, 647.9), 21);
  EXPECT_EQ(_fp->subregion3(22.15e6, 647.5), 22);
  EXPECT_EQ(_fp->subregion3(22.3e6, 648.1), 22);
  EXPECT_EQ(_fp->subregion3(22.11e6, 648.0), 23);
  EXPECT_EQ(_fp->subregion3(22.3e6, 649.0), 23);
  EXPECT_EQ(_fp->subregion3(22.0e6, 646.84), 24);
  EXPECT_EQ(_fp->subregion3(22.064e6, 647.05), 24);
  EXPECT_EQ(_fp->subregion3(22.0e6, 646.89), 25);
  EXPECT_EQ(_fp->subregion3(22.064e6, 647.15), 25);
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
  REL_TEST("rho", _fp->densityRegion3(50.0e6, 630.0), 1.0 / 0.001470853100, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(80.0e6, 670.0), 1.0 / 0.001503831359, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(50.0e6, 710.0), 1.0 / 0.002204728587, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(80.0e6, 750.0), 1.0 / 0.001973692940, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(20.0e6, 630.0), 1.0 / 0.001761696406, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(30.0e6, 650.0), 1.0 / 0.001819560617, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(26.0e6, 656.0), 1.0 / 0.002245587720, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(30.0e6, 670.0), 1.0 / 0.002506897702, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(26.0e6, 661.0), 1.0 / 0.002970225962, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(30.0e6, 675.0), 1.0 / 0.003004627086, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(26.0e6, 671.0), 1.0 / 0.005019029401, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(30.0e6, 690.0), 1.0 / 0.004656470142, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(23.6e6, 649.0), 1.0 / 0.002163198378, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(24.0e6, 650.0), 1.0 / 0.002166044161, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(23.6e6, 652.0), 1.0 / 0.002651081407, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(24.0e6, 654.0), 1.0 / 0.002967802335, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(23.6e6, 653.0), 1.0 / 0.003273916816, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(24.0e6, 655.0), 1.0 / 0.003550329864, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(23.5e6, 655.0), 1.0 / 0.004545001142, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(24.0e6, 660.0), 1.0 / 0.005100267704, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(23.0e6, 660.0), 1.0 / 0.006109525997, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(24.0e6, 670.0), 1.0 / 0.006427325645, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.6e6, 646.0), 1.0 / 0.002117860851, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(23.0e6, 646.0), 1.0 / 0.002062374674, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.6e6, 648.6), 1.0 / 0.002533063780, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.8e6, 649.3), 1.0 / 0.002572971781, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.6e6, 649.0), 1.0 / 0.002923432711, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.8e6, 649.7), 1.0 / 0.002913311494, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.6e6, 649.1), 1.0 / 0.003131208996, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.8e6, 649.9), 1.0 / 0.003221160278, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.6e6, 649.4), 1.0 / 0.003715596186, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.8e6, 650.2), 1.0 / 0.003664754790, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(21.1e6, 640.0), 1.0 / 0.001970999272, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(21.8e6, 643.0), 1.0 / 0.002043919161, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(21.1e6, 644.0), 1.0 / 0.005251009921, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(21.8e6, 648.0), 1.0 / 0.005256844741, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(19.1e6, 635.0), 1.0 / 0.001932829079, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(20.0e6, 638.0), 1.0 / 0.001985387227, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(17.0e6, 626.0), 1.0 / 0.008483262001, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(20.0e6, 640.0), 1.0 / 0.006227528101, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(21.5e6, 644.6), 1.0 / 0.002268366647, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.0e6, 646.1), 1.0 / 0.002296350553, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.5e6, 648.6), 1.0 / 0.002832373260, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.3e6, 647.9), 1.0 / 0.002811424405, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.15e6, 647.5), 1.0 / 0.003694032281, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.3e6, 648.1), 1.0 / 0.003622226305, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.11e6, 648.0), 1.0 / 0.004528072649, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.3e6, 649.0), 1.0 / 0.004556905799, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.0e6, 646.84), 1.0 / 0.002698354719, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.064e6, 647.05), 1.0 / 0.002717655648, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.0e6, 646.89), 1.0 / 0.003798732962, 1.0e-8);
  REL_TEST("rho", _fp->densityRegion3(22.064e6, 647.15), 1.0 / 0.003701940010, 1.0e-8);
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

  // Region 1 properties
  p0 = 3.0e6;
  p1 = 80.0e6;
  p2 = 3.0e6;
  T0 = 300.0;
  T1 = 300.0;
  T2 = 500.0;

  REL_TEST("rho", _fp->rho(p0, T0), 1.0 / 0.00100215168, 1.0e-8);
  REL_TEST("rho", _fp->rho(p1, T1), 1.0 / 0.000971180894, 1.0e-8);
  REL_TEST("rho", _fp->rho(p2, T2), 1.0 / 0.00120241800, 1.0e-8);
  REL_TEST("h", _fp->h(p0, T0), 115.331273e3, 1.0e-8);
  REL_TEST("h", _fp->h(p1, T1), 184.142828e3, 1.0e-8);
  REL_TEST("h", _fp->h(p2, T2), 975.542239e3, 1.0e-8);
  REL_TEST("e", _fp->e(p0, T0), 112.324818e3, 1.0e-8);
  REL_TEST("e", _fp->e(p1, T1), 106.448356e3, 1.0e-8);
  REL_TEST("e", _fp->e(p2, T2), 971.934985e3, 1.0e-8);
  REL_TEST("s", _fp->s(p0, T0), 0.392294792e3, 1.0e-8);
  REL_TEST("s", _fp->s(p1, T1), 0.368563852e3, 1.0e-8);
  REL_TEST("s", _fp->s(p2, T2), 2.58041912e3, 1.0e-8);
  REL_TEST("cp", _fp->cp(p0, T0), 4.17301218e3, 1.0e-8);
  REL_TEST("cp", _fp->cp(p1, T1), 4.01008987e3, 1.0e-8);
  REL_TEST("cp", _fp->cp(p2, T2), 4.65580682e3, 1.0e-8);
  REL_TEST("c", _fp->c(p0, T0), 1507.73921, 1.0e-8);
  REL_TEST("c", _fp->c(p1, T1), 1634.69054, 1.0e-8);
  REL_TEST("c", _fp->c(p2, T2), 1240.71337, 1.0e-8);

  // Region 2 properties
  p0 = 3.5e3;
  p1 = 3.5e3;
  p2 = 30.0e6;
  T0 = 300.0;
  T1 = 700.0;
  T2 = 700.0;

  REL_TEST("rho", _fp->rho(p0, T0), 1.0 / 39.4913866, 1.0e-8);
  REL_TEST("rho", _fp->rho(p1, T1), 1.0 / 92.3015898, 1.0e-8);
  REL_TEST("rho", _fp->rho(p2, T2), 1.0 / 0.00542946619, 1.0e-8);
  REL_TEST("h", _fp->h(p0, T0), 2549.91145e3, 1.0e-8);
  REL_TEST("h", _fp->h(p1, T1), 3335.68375e3, 1.0e-8);
  REL_TEST("h", _fp->h(p2, T2), 2631.49474e3, 1.0e-8);
  REL_TEST("e", _fp->e(p0, T0), 2411.6916e3, 1.0e-8);
  REL_TEST("e", _fp->e(p1, T1), 3012.62819e3, 1.0e-8);
  REL_TEST("e", _fp->e(p2, T2), 2468.61076e3, 1.0e-8);
  REL_TEST("s", _fp->s(p0, T0), 8.52238967e3, 1.0e-8);
  REL_TEST("s", _fp->s(p1, T1), 10.1749996e3, 1.0e-8);
  REL_TEST("s", _fp->s(p2, T2), 5.17540298e3, 1.0e-8);
  REL_TEST("cp", _fp->cp(p0, T0), 1.91300162e3, 1.0e-8);
  REL_TEST("cp", _fp->cp(p1, T1), 2.08141274e3, 1.0e-8);
  REL_TEST("cp", _fp->cp(p2, T2), 10.3505092e3, 1.0e-8);
  REL_TEST("c", _fp->c(p0, T0), 427.920172, 1.0e-8);
  REL_TEST("c", _fp->c(p1, T1), 644.289068, 1.0e-8);
  REL_TEST("c", _fp->c(p2, T2), 480.386523, 1.0e-8);

  // Region 3 properties
  p0 = 25.5837018e6;
  p1 = 22.2930643e6;
  p2 = 78.3095639e6;
  T0 = 650.0;
  T1 = 650.0;
  T2 = 750.0;

  // Note: lower tolerance in this region as density is calculated using backwards equation
  REL_TEST("rho", _fp->rho(p0, T0), 500.0, 1.0e-5);
  REL_TEST("rho", _fp->rho(p1, T1), 200.0, 1.0e-5);
  REL_TEST("rho", _fp->rho(p2, T2), 500.0, 1.0e-5);
  REL_TEST("h", _fp->h(p0, T0), 1863.43019e3, 1.0e-5);
  REL_TEST("h", _fp->h(p1, T1), 2375.12401e3, 1.0e-5);
  REL_TEST("h", _fp->h(p2, T2), 2258.68845e3, 1.0e-5);
  REL_TEST("e", _fp->e(p0, T0), 1812.26279e3, 1.0e-5);
  REL_TEST("e", _fp->e(p1, T1), 2263.65868e3, 1.0e-5);
  REL_TEST("e", _fp->e(p2, T2), 2102.06932e3, 1.0e-5);
  REL_TEST("s", _fp->s(p0, T0), 4.05427273e3, 1.0e-5);
  REL_TEST("s", _fp->s(p1, T1), 4.85438792e3, 1.0e-5);
  REL_TEST("s", _fp->s(p2, T2), 4.46971906e3, 1.0e-5);
  REL_TEST("cp", _fp->cp(p0, T0), 13.8935717e3, 1.0e-4);
  REL_TEST("cp", _fp->cp(p1, T1), 44.6579342e3, 1.0e-5);
  REL_TEST("cp", _fp->cp(p2, T2), 6.34165359e3, 1.0e-5);
  REL_TEST("c", _fp->c(p0, T0), 502.005554, 1.0e-5);
  REL_TEST("c", _fp->c(p1, T1), 383.444594, 1.0e-5);
  REL_TEST("c", _fp->c(p2, T2), 760.696041, 1.0e-5);

  // Region 5 properties
  p0 = 0.5e6;
  p1 = 30.0e6;
  p2 = 30.0e6;
  T0 = 1500.0;
  T1 = 1500.0;
  T2 = 2000.0;

  REL_TEST("rho", _fp->rho(p0, T0), 1.0 / 1.38455090, 1.0e-8);
  REL_TEST("rho", _fp->rho(p1, T1), 1.0 / 0.0230761299, 1.0e-8);
  REL_TEST("rho", _fp->rho(p2, T2), 1.0 / 0.0311385219, 1.0e-8);
  REL_TEST("h", _fp->h(p0, T0), 5219.76855e3, 1.0e-8);
  REL_TEST("h", _fp->h(p1, T1), 5167.23514e3, 1.0e-8);
  REL_TEST("h", _fp->h(p2, T2), 6571.22604e3, 1.0e-8);
  REL_TEST("e", _fp->e(p0, T0), 4527.4931e3, 1.0e-8);
  REL_TEST("e", _fp->e(p1, T1), 4474.95124e3, 1.0e-8);
  REL_TEST("e", _fp->e(p2, T2), 5637.07038e3, 1.0e-8);
  REL_TEST("s", _fp->s(p0, T0), 9.65408875e3, 1.0e-8);
  REL_TEST("s", _fp->s(p1, T1), 7.72970133e3, 1.0e-8);
  REL_TEST("s", _fp->s(p2, T2), 8.53640523e3, 1.0e-8);
  REL_TEST("cp", _fp->cp(p0, T0), 2.61609445e3, 1.0e-8);
  REL_TEST("cp", _fp->cp(p1, T1), 2.72724317e3, 1.0e-8);
  REL_TEST("cp", _fp->cp(p2, T2), 2.88569882e3, 1.0e-8);
  REL_TEST("c", _fp->c(p0, T0), 917.06869, 1.0e-8);
  REL_TEST("c", _fp->c(p1, T1), 928.548002, 1.0e-8);
  REL_TEST("c", _fp->c(p2, T2), 1067.36948, 1.0e-8);

  // Viscosity
  REL_TEST("mu", _fp->mu_from_rho_T(998.0, 298.15), 889.735100e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu_from_rho_T(1200.0, 298.15), 1437.649467e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu_from_rho_T(1000.0, 373.15), 307.883622e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu_from_rho_T(1.0, 433.15), 14.538324e-6, 1.0e-7);
  REL_TEST("mu", _fp->mu_from_rho_T(1000.0, 433.15), 217.685358e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu_from_rho_T(1.0, 873.15), 32.619287e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu_from_rho_T(100.0, 873.15), 35.802262e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu_from_rho_T(600.0, 873.15), 77.430195e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu_from_rho_T(1.0, 1173.15), 44.217245e-6, 1.0e-7);
  REL_TEST("mu", _fp->mu_from_rho_T(100.0, 1173.15), 47.640433e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu_from_rho_T(400.0, 1173.15), 64.154608e-6, 1.0e-8);

  ABS_TEST("mu", _fp->mu(1e6, 298.15), 889.898581797e-6, 2e-8);
  ABS_TEST("mu", _fp->mu(2e6, 298.15), 889.763899645e-6, 1e-8);
  ABS_TEST("mu", _fp->mu(1e6, 373.15), 281.825180491e-6, 1e-8);
  ABS_TEST("mu", _fp->mu(2e6, 373.15), 282.09550632e-6, 1e-8);
  ABS_TEST("mu", _fp->mu(1e6, 433.15), 170.526801634e-6, 1e-8);
  ABS_TEST("mu", _fp->mu(2e6, 433.15), 170.780193827e-6, 1e-8);
  ABS_TEST("mu", _fp->mu(1e6, 873.15), 3.2641885983e-5, 1e-12);
  ABS_TEST("mu", _fp->mu(2e6, 873.15), 3.26820969808e-5, 1e-12);
  ABS_TEST("mu", _fp->mu(1e6, 1173.15), 4.42374919686e-5, 1e-12);
  ABS_TEST("mu", _fp->mu(2e6, 1173.15), 4.42823959629e-5, 1e-12);

  // Thermal conductivity
  // Note: data is given for pressure and temperature, but k requires density
  // and temperature
  REL_TEST("k", _fp->k_from_rho_T(_fp->rho(1.0e6, 323.15), 323.15), 0.641, 1.0e-4);
  REL_TEST("k", _fp->k_from_rho_T(_fp->rho(20.0e6, 623.15), 623.15), 0.4541, 1.0e-4);
  REL_TEST("k", _fp->k_from_rho_T(_fp->rho(50.0e6, 773.15), 773.15), 0.2055, 1.0e-4);

  ABS_TEST("k", _fp->k(1.0e6, 323.15), 0.640972, 5e-7);
  ABS_TEST("k", _fp->k(20.0e6, 623.15), 0.454131, 7e-7);
  ABS_TEST("k", _fp->k(50.0e6, 773.15), 0.205485, 5e-7);

  // Backwards equation T(p,h)
  // Region 1
  REL_TEST("T(p,h)", _fp->temperature_from_ph(3.0e6, 500.0e3), 0.391798509e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(80.0e6, 500.0e3), 0.378108626e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(80.0e6, 1500.0e3), 0.611041229e3, 1.0e-8);

  // Region 2 (subregion a)
  REL_TEST("T(p,h)", _fp->temperature_from_ph(1.0e3, 3000.0e3), 0.534433241e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(3.0e6, 3000.0e3), 0.575373370e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(3.0e6, 4000.0e3), 0.101077577e4, 1.0e-8);

  // Region 2 (subregion b)
  REL_TEST("T(p,h)", _fp->temperature_from_ph(5.0e6, 3500.0e3), 0.801299102e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(5.0e6, 4000.0e3), 0.101531583e4, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(25.0e6, 3500.0e3), 0.875279054e3, 1.0e-8);

  // Region 2 (subregion c)
  REL_TEST("T(p,h)", _fp->temperature_from_ph(40.0e6, 2700.0e3), 0.743056411e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(60.0e6, 2700.0e3), 0.791137067e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(60.0e6, 3200.0e3), 0.882756860e3, 1.0e-8);

  // Region 3 (subregion a)
  REL_TEST("T(p,h)", _fp->temperature_from_ph(20.0e6, 1700.0e3), 0.6293083892e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(50.0e6, 2000.0e3), 0.6905718338e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(100.0e6, 2100.0e3), 0.7336163014e3, 1.0e-8);

  // Region 3 (subregion b)
  REL_TEST("T(p,h)", _fp->temperature_from_ph(20.0e6, 2500.0e3), 0.6418418053e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(50.0e6, 2400.0e3), 0.7351848618e3, 1.0e-8);
  REL_TEST("T(p,h)", _fp->temperature_from_ph(100.0e6, 2700.0e3), 0.8420460876e3, 1.0e-8);
}

/**
 * Verify calculation of the derivatives in all regions by comparing with finite
 * differences
 */
TEST_F(Water97FluidPropertiesTest, derivatives)
{
  // Region 1
  Real p = 3.0e6;
  Real T = 300.0;
  regionDerivatives(p, T, 1.0e-6);

  // Region 2
  p = 3.5e3;
  T = 300.0;
  regionDerivatives(p, T, 1.0e-6);

  // Region 3
  p = 26.0e6;
  T = 650.0;
  regionDerivatives(p, T, 1.0e-2);

  // Region 4 (saturation curve)
  T = 300.0;
  Real dT = 1.0e-4;

  Real dpSat_dT_fd = (_fp->vaporPressure(T + dT) - _fp->vaporPressure(T - dT)) / (2.0 * dT);
  Real pSat = 0.0, dpSat_dT = 0.0;
  _fp->vaporPressure_dT(T, pSat, dpSat_dT);

  REL_TEST("dvaporPressure_dT", dpSat_dT, dpSat_dT_fd, 1.0e-6);

  // Region 5
  p = 30.0e6;
  T = 1500.0;
  regionDerivatives(p, T, 1.0e-6);

  // Viscosity
  Real rho = 998.0, drho_dp = 0.0, drho_dT = 0.0;
  T = 298.15;
  Real drho = 1.0e-4;

  Real dmu_drho_fd =
      (_fp->mu_from_rho_T(rho + drho, T) - _fp->mu_from_rho_T(rho - drho, T)) / (2.0 * drho);
  Real mu = 0.0, dmu_drho = 0.0, dmu_dT = 0.0;
  _fp->mu_drhoT_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);

  ABS_TEST("mu", mu, _fp->mu_from_rho_T(rho, T), 1.0e-15);
  REL_TEST("dmu_dp", dmu_drho, dmu_drho_fd, 1.0e-6);

  // To properly test derivative wrt temperature, use p and T and calculate density,
  // so that the change in density wrt temperature is included
  p = 1.0e6;
  dT = 1.0e-4;
  _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);
  _fp->mu_drhoT_from_rho_T(rho, T, drho_dT, mu, dmu_drho, dmu_dT);
  Real dmu_dT_fd = (_fp->mu_from_rho_T(_fp->rho(p, T + dT), T + dT) -
                    _fp->mu_from_rho_T(_fp->rho(p, T - dT), T - dT)) /
                   (2.0 * dT);

  REL_TEST("dmu_dT", dmu_dT, dmu_dT_fd, 1.0e-6);
}
