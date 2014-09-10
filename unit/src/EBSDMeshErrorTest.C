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

#include "EBSDMeshErrorTest.h"

//Moose includes
#include "EBSDMesh.h"
#include "InputParameters.h"
#include "MooseParsedFunction.h"
#include "FEProblem.h"
#include "MarmotUnitApp.h"
#include "AppFactory.h"
#include "GeneratedMesh.h"

CPPUNIT_TEST_SUITE_REGISTRATION( EBSDMeshErrorTest );


void
EBSDMeshErrorTest::setUp()
{
  const char *argv[2] = { "foo", "\0" };
  _app = AppFactory::createApp("MarmotUnitApp", 1, (char**)argv);
  _factory = &_app->getFactory();
}

void
EBSDMeshErrorTest::tearDown()
{
  delete _app;
  _app = NULL;
}


void
EBSDMeshErrorTest::fileDoesNotExist()
{
  // generate input parameter set
  InputParameters params = _factory->getValidParams("EBSDMesh");
  params.addPrivateParam("_moose_app", _app);

  // set filename
  params.set<FileName>("filename") = "FILEDOESNOTEXIST";

  // construct mesh object
  EBSDMesh * mesh = new EBSDMesh("unit_test_mesh", params);

  try
  {
    // trigger mesh building with invalid EBSD filename
    mesh->buildMesh();
  }
  catch(const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT( msg.find("Can't open EBSD file: FILEDOESNOTEXIST") != std::string::npos );
  }

  // delete mesh object
  delete mesh;
}

void
EBSDMeshErrorTest::geometrySpecifiedError()
{
  // test all these Real parameters
  const unsigned int nreal = 6;
  const char * real_params[nreal] = {"xmin", "xmax", "ymin", "ymax", "zmin", "zmax"};
  testParam<Real>(nreal, real_params);

  // test all these int parameters
  const unsigned int nint = 3;
  const char * int_params[nint] = {"nx", "ny", "nz"};
  testParam<int>(nint, int_params);
}

template<typename T>
void
EBSDMeshErrorTest::testParam(unsigned int nparam, const char ** param_list)
{
  for (unsigned int i = 0; i < nparam; ++i)
  {
    // generate input parameter set
    InputParameters params = _factory->getValidParams("EBSDMesh");
    params.addPrivateParam("_moose_app", _app);

    // set a single parameter
    params.set<T>(param_list[i]) = T(1.0);

    try
    {
      // construct mesh object
      EBSDMesh * mesh = new EBSDMesh("unit_test_mesh", params);
      delete mesh;
    }
    catch(const std::exception & e)
    {
      std::string msg(e.what());
      CPPUNIT_ASSERT( msg.find("Do not specify mesh geometry information, it is read from the EBSD file.") != std::string::npos );
    }
  }
}
