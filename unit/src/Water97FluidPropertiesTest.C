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

#include "MooseApp.h"
#include "Utils.h"
#include "Water97FluidPropertiesTest.h"

#include "FEProblem.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"
#include "Water97FluidProperties.h"

CPPUNIT_TEST_SUITE_REGISTRATION(Water97FluidPropertiesTest);

void
Water97FluidPropertiesTest::registerObjects(Factory & factory)
{
  registerUserObject(Water97FluidProperties);
}

void
Water97FluidPropertiesTest::buildObjects()
{
  InputParameters mesh_params = _factory->getValidParams("GeneratedMesh");
  mesh_params.set<MooseEnum>("dim") = "3";
  mesh_params.set<std::string>("name") = "mesh";
  mesh_params.set<std::string>("_object_name") = "name1";
  _mesh = new GeneratedMesh(mesh_params);

  InputParameters problem_params = _factory->getValidParams("FEProblem");
  problem_params.set<MooseMesh *>("mesh") = _mesh;
  problem_params.set<std::string>("name") = "problem";
  problem_params.set<std::string>("_object_name") = "name2";
  _fe_problem = new FEProblem(problem_params);

  InputParameters uo_pars = _factory->getValidParams("Water97FluidProperties");
  _fe_problem->addUserObject("Water97FluidProperties", "fp", uo_pars);
  _fp = &_fe_problem->getUserObject<Water97FluidProperties>("fp");
}

void
Water97FluidPropertiesTest::setUp()
{
  char str[] = "foo";
  char * argv[] = {str, NULL};

  _app = AppFactory::createApp("MooseUnitApp", 1, (char **)argv);
  _factory = &_app->getFactory();

  registerObjects(*_factory);
  buildObjects();
}

void
Water97FluidPropertiesTest::tearDown()
{
  delete _fe_problem;
  delete _mesh;
  delete _app;
}

void
Water97FluidPropertiesTest::inRegion()
{
  // Region 1
  CPPUNIT_ASSERT(_fp->inRegion(3.0e6, 300) == 1);
  CPPUNIT_ASSERT(_fp->inRegion(80.0e6, 300) == 1);
  CPPUNIT_ASSERT(_fp->inRegion(3.0e6, 500) == 1);

  // Region 2
  CPPUNIT_ASSERT(_fp->inRegion(3.5e3, 300) == 2);
  CPPUNIT_ASSERT(_fp->inRegion(30.0e6, 700) == 2);
  CPPUNIT_ASSERT(_fp->inRegion(30.0e6, 700) == 2);

  // Region 3
  CPPUNIT_ASSERT(_fp->inRegion(25.588e6, 650) == 3);
  CPPUNIT_ASSERT(_fp->inRegion(22.298e6, 650) == 3);
  CPPUNIT_ASSERT(_fp->inRegion(78.32e6, 750) == 3);

  // Region 5
  CPPUNIT_ASSERT(_fp->inRegion(0.5e6, 1500) == 5);
  CPPUNIT_ASSERT(_fp->inRegion(30.0e6, 1500) == 5);
  CPPUNIT_ASSERT(_fp->inRegion(30.0e6, 2000) == 5);

  // Test out of range errors
  unsigned int region;
  try
  {
    // Trigger invalid pressure error
    region = _fp->inRegion(101.0e6, 300.0);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(
        msg.find("Pressure 1.01e+08 is out of range in Water97FluidProperties::inRegion") !=
        std::string::npos);
  }

  try
  {
    // Trigger another invalid pressure error
    region = _fp->inRegion(51.0e6, 1200.0);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(
        msg.find("Pressure 5.1e+07 is out of range in Water97FluidProperties::inRegion") !=
        std::string::npos);
  }

  try
  {
    // Trigger invalid temperature error
    region = _fp->inRegion(5.0e6, 2001.0);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(
        msg.find("Temperature 2001 is out of range in Water97FluidProperties::inRegion") !=
        std::string::npos);
  }
}

void
Water97FluidPropertiesTest::b23()
{
  REL_TEST("b23T", _fp->b23T(16.5291643e6), 623.15, 1.0e-8);
  REL_TEST("b23p", _fp->b23p(623.15), 16.5291643e6, 1.0e-8);
}

void
Water97FluidPropertiesTest::pSat()
{
  REL_TEST("pSat", _fp->pSat(300), 3.53658941e3, 1.0e-8);
  REL_TEST("pSat", _fp->pSat(500), 2.63889776e6, 1.0e-8);
  REL_TEST("pSat", _fp->pSat(600), 12.3443146e6, 1.0e-8);
}

void
Water97FluidPropertiesTest::TSat()
{
  REL_TEST("pSat", _fp->TSat(0.1e6), 372.755919, 1.0e-8);
  REL_TEST("pSat", _fp->TSat(1.0e6), 453.035632, 1.0e-8);
  REL_TEST("pSat", _fp->TSat(10.0e6), 584.149488, 1.0e-8);
}

void
Water97FluidPropertiesTest::subregion3()
{
  CPPUNIT_ASSERT(_fp->subregion3(50.0e6, 630.0) == 0);
  CPPUNIT_ASSERT(_fp->subregion3(80.0e6, 670.0) == 0);
  CPPUNIT_ASSERT(_fp->subregion3(50.0e6, 710.0) == 1);
  CPPUNIT_ASSERT(_fp->subregion3(80.0e6, 750.0) == 1);
  CPPUNIT_ASSERT(_fp->subregion3(20.0e6, 630.0) == 2);
  CPPUNIT_ASSERT(_fp->subregion3(30.0e6, 650.0) == 2);
  CPPUNIT_ASSERT(_fp->subregion3(26.0e6, 656.0) == 3);
  CPPUNIT_ASSERT(_fp->subregion3(30.0e6, 670.0) == 3);
  CPPUNIT_ASSERT(_fp->subregion3(26.0e6, 661.0) == 4);
  CPPUNIT_ASSERT(_fp->subregion3(30.0e6, 675.0) == 4);
  CPPUNIT_ASSERT(_fp->subregion3(26.0e6, 671.0) == 5);
  CPPUNIT_ASSERT(_fp->subregion3(30.0e6, 690.0) == 5);
  CPPUNIT_ASSERT(_fp->subregion3(23.6e6, 649.0) == 6);
  CPPUNIT_ASSERT(_fp->subregion3(24.0e6, 650.0) == 6);
  CPPUNIT_ASSERT(_fp->subregion3(23.6e6, 652.0) == 7);
  CPPUNIT_ASSERT(_fp->subregion3(24.0e6, 654.0) == 7);
  CPPUNIT_ASSERT(_fp->subregion3(23.6e6, 653.0) == 8);
  CPPUNIT_ASSERT(_fp->subregion3(24.0e6, 655.0) == 8);
  CPPUNIT_ASSERT(_fp->subregion3(23.5e6, 655.0) == 9);
  CPPUNIT_ASSERT(_fp->subregion3(24.0e6, 660.0) == 9);
  CPPUNIT_ASSERT(_fp->subregion3(23.0e6, 660.0) == 10);
  CPPUNIT_ASSERT(_fp->subregion3(24.0e6, 670.0) == 10);
  CPPUNIT_ASSERT(_fp->subregion3(22.6e6, 646.0) == 11);
  CPPUNIT_ASSERT(_fp->subregion3(23.0e6, 646.0) == 11);
  CPPUNIT_ASSERT(_fp->subregion3(22.6e6, 648.6) == 12);
  CPPUNIT_ASSERT(_fp->subregion3(22.8e6, 649.3) == 12);
  CPPUNIT_ASSERT(_fp->subregion3(22.6e6, 649.0) == 13);
  CPPUNIT_ASSERT(_fp->subregion3(22.8e6, 649.7) == 13);
  CPPUNIT_ASSERT(_fp->subregion3(22.6e6, 649.1) == 14);
  CPPUNIT_ASSERT(_fp->subregion3(22.8e6, 649.9) == 14);
  CPPUNIT_ASSERT(_fp->subregion3(22.6e6, 649.4) == 15);
  CPPUNIT_ASSERT(_fp->subregion3(22.8e6, 650.2) == 15);
  CPPUNIT_ASSERT(_fp->subregion3(21.1e6, 640.0) == 16);
  CPPUNIT_ASSERT(_fp->subregion3(21.8e6, 643.0) == 16);
  CPPUNIT_ASSERT(_fp->subregion3(21.1e6, 644.0) == 17);
  CPPUNIT_ASSERT(_fp->subregion3(21.8e6, 648.0) == 17);
  CPPUNIT_ASSERT(_fp->subregion3(19.1e6, 635.0) == 18);
  CPPUNIT_ASSERT(_fp->subregion3(20.0e6, 638.0) == 18);
  CPPUNIT_ASSERT(_fp->subregion3(17.0e6, 626.0) == 19);
  CPPUNIT_ASSERT(_fp->subregion3(20.0e6, 640.0) == 19);
  CPPUNIT_ASSERT(_fp->subregion3(21.5e6, 644.6) == 20);
  CPPUNIT_ASSERT(_fp->subregion3(22.0e6, 646.1) == 20);
  CPPUNIT_ASSERT(_fp->subregion3(22.5e6, 648.6) == 21);
  CPPUNIT_ASSERT(_fp->subregion3(22.3e6, 647.9) == 21);
  CPPUNIT_ASSERT(_fp->subregion3(22.15e6, 647.5) == 22);
  CPPUNIT_ASSERT(_fp->subregion3(22.3e6, 648.1) == 22);
  CPPUNIT_ASSERT(_fp->subregion3(22.11e6, 648.0) == 23);
  CPPUNIT_ASSERT(_fp->subregion3(22.3e6, 649.0) == 23);
  CPPUNIT_ASSERT(_fp->subregion3(22.0e6, 646.84) == 24);
  CPPUNIT_ASSERT(_fp->subregion3(22.064e6, 647.05) == 24);
  CPPUNIT_ASSERT(_fp->subregion3(22.0e6, 646.89) == 25);
  CPPUNIT_ASSERT(_fp->subregion3(22.064e6, 647.15) == 25);
}

void
Water97FluidPropertiesTest::subregion3Density()
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

void
Water97FluidPropertiesTest::properties()
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
  REL_TEST("mu", _fp->mu(998.0, 298.15), 889.735100e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu(1200.0, 298.15), 1437.649467e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu(1000.0, 373.15), 307.883622e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu(1.0, 433.15), 14.538324e-6, 1.0e-7);
  REL_TEST("mu", _fp->mu(1000.0, 433.15), 217.685358e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu(1.0, 873.15), 32.619287e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu(100.0, 873.15), 35.802262e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu(600.0, 873.15), 77.430195e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu(1.0, 1173.15), 44.217245e-6, 1.0e-7);
  REL_TEST("mu", _fp->mu(100.0, 1173.15), 47.640433e-6, 1.0e-8);
  REL_TEST("mu", _fp->mu(400.0, 1173.15), 64.154608e-6, 1.0e-8);

  // Thermal conductivity
  // Note: data is given for pressure and temperature, but k requires density
  // and temperature
  REL_TEST("k", _fp->k(_fp->rho(1.0e6, 323.15), 323.15), 0.641, 1.0e-4);
  REL_TEST("k", _fp->k(_fp->rho(20.0e6, 623.15), 623.15), 0.4541, 1.0e-4);
  REL_TEST("k", _fp->k(_fp->rho(50.0e6, 773.15), 773.15), 0.2055, 1.0e-4);
}

void
Water97FluidPropertiesTest::derivatives()
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

  Real dpSat_dT_fd = (_fp->pSat(T + dT) - _fp->pSat(T - dT)) / (2.0 * dT);
  Real pSat = 0.0, dpSat_dT = 0.0;
  _fp->pSat_dT(T, pSat, dpSat_dT);

  REL_TEST("dpSat_dT", dpSat_dT, dpSat_dT_fd, 1.0e-6);

  // Region 5
  p = 30.0e6;
  T = 1500.0;
  regionDerivatives(p, T, 1.0e-6);

  // Viscosity
  Real rho = 998.0;
  T = 298.15;
  Real drho = 1.0e-4;
  dT = 1.0e-4;

  Real dmu_drho_fd = (_fp->mu(rho + drho, T) - _fp->mu(rho - drho, T)) / (2.0 * drho);
  Real dmu_dT_fd = (_fp->mu(rho, T + dT) - _fp->mu(rho, T - dT)) / (2.0 * dT);
  Real mu = 0.0, dmu_drho = 0.0, dmu_dT = 0.0;
  _fp->mu_drhoT(rho, T, mu, dmu_drho, dmu_dT);

  ABS_TEST("mu", mu, _fp->mu(rho, T), 1.0e-15);
  REL_TEST("dmu_dp", dmu_drho, dmu_drho_fd, 1.0e-6);
  REL_TEST("dmu_dT", dmu_dT, dmu_dT_fd, 1.0e-6);
}

void
Water97FluidPropertiesTest::regionDerivatives(Real p, Real T, Real tol)
{
  // Finite differencing parameters
  Real dp = 1.0e1;
  Real dT = 1.0e-4;

  // density
  Real drho_dp_fd = (_fp->rho(p + dp, T) - _fp->rho(p - dp, T)) / (2.0 * dp);
  Real drho_dT_fd = (_fp->rho(p, T + dT) - _fp->rho(p, T - dT)) / (2.0 * dT);
  Real rho = 0.0, drho_dp = 0.0, drho_dT = 0.0;
  _fp->rho_dpT(p, T, rho, drho_dp, drho_dT);

  ABS_TEST("rho", rho, _fp->rho(p, T), 1.0e-15);
  REL_TEST("drho_dp", drho_dp, drho_dp_fd, tol);
  REL_TEST("drho_dT", drho_dT, drho_dT_fd, tol);

  // enthalpy
  Real dh_dp_fd = (_fp->h(p + dp, T) - _fp->h(p - dp, T)) / (2.0 * dp);
  Real dh_dT_fd = (_fp->h(p, T + dT) - _fp->h(p, T - dT)) / (2.0 * dT);
  Real h = 0.0, dh_dp = 0.0, dh_dT = 0.0;
  _fp->h_dpT(p, T, h, dh_dp, dh_dT);

  ABS_TEST("h", h, _fp->h(p, T), 1.0e-15);
  REL_TEST("dh_dp", dh_dp, dh_dp_fd, tol);
  REL_TEST("dh_dT", dh_dT, dh_dT_fd, tol);

  // internal energy
  Real de_dp_fd = (_fp->e(p + dp, T) - _fp->e(p - dp, T)) / (2.0 * dp);
  Real de_dT_fd = (_fp->e(p, T + dT) - _fp->e(p, T - dT)) / (2.0 * dT);
  Real e = 0.0, de_dp = 0.0, de_dT = 0.0;
  _fp->e_dpT(p, T, e, de_dp, de_dT);

  ABS_TEST("e", e, _fp->e(p, T), 1.0e-15);
  REL_TEST("de_dp", de_dp, de_dp_fd, tol);
  REL_TEST("de_dT", de_dT, de_dT_fd, tol);
}
