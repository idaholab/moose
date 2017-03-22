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

// Moose includes
#include "EBSDMesh.h"
#include "InputParameters.h"
#include "MooseParsedFunction.h"
#include "MooseUnitApp.h"
#include "AppFactory.h"

CPPUNIT_TEST_SUITE_REGISTRATION(EBSDMeshErrorTest);

void
EBSDMeshErrorTest::setUp()
{
  const char * argv[2] = {"foo", "\0"};
  _app = AppFactory::createApp("MooseUnitApp", 1, (char **)argv);
  _factory = &_app->getFactory();
}

void
EBSDMeshErrorTest::tearDown()
{
  delete _app;
}

void
EBSDMeshErrorTest::fileDoesNotExist()
{
  // generate input parameter set
  InputParameters params = validParams<EBSDMesh>();
  params.addPrivateParam("_moose_app", _app);
  params.set<std::string>("_object_name", "EBSD");

  // set filename
  params.set<FileName>("filename") = "FILEDOESNOTEXIST";

  // construct mesh object
  std::unique_ptr<EBSDMesh> mesh(new EBSDMesh(params));

  try
  {
    // trigger mesh building with invalid EBSD filename
    mesh->buildMesh();
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(msg.find("Can't open EBSD file: FILEDOESNOTEXIST") != std::string::npos);
  }
}

void
EBSDMeshErrorTest::headerError()
{
  const unsigned int ntestcase = 4;

  const char * testcase[ntestcase][2] = {
      {"data/ebsd/ebsd3D_zerostep.txt", "Error reading header, EBSD data step size is zero."},
      {"data/ebsd/ebsd3D_zerosize.txt", "Error reading header, EBSD grid size is zero."},
      {"data/ebsd/ebsd3D_zerodim.txt", "Error reading header, EBSD data is zero dimensional."},
      {"data/ebsd/ebsd3D_norefine.txt",
       "EBSDMesh error. Requested uniform_refine levels not possible."},
  };

  for (unsigned int i = 0; i < ntestcase; ++i)
    headerErrorHelper(testcase[i][0], testcase[i][1]);
}

void
EBSDMeshErrorTest::headerErrorHelper(const char * filename, const char * error)
{
  // generate input parameter set
  InputParameters params = validParams<EBSDMesh>();
  params.addPrivateParam("_moose_app", _app);
  params.set<std::string>("_object_name") = filename; // use the filename to define a unique name

  // set filename
  params.set<FileName>("filename") = filename;
  params.set<unsigned int>("uniform_refine") = 2;

  // construct mesh object
  std::unique_ptr<EBSDMesh> mesh(new EBSDMesh(params));

  try
  {
    // trigger mesh building with invalid EBSD filename
    mesh->buildMesh();
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT_MESSAGE(filename, msg.find(error) != std::string::npos);
  }
}

void
EBSDMeshErrorTest::geometrySpecifiedError()
{
  // test all these Real parameters
  const unsigned int nreal = 6;
  const char * real_params[nreal] = {"xmin", "xmax", "ymin", "ymax", "zmin", "zmax"};
  testParam<Real>(nreal, real_params, "TestA");

  // test all these int parameters
  const unsigned int nint = 3;
  const char * int_params[nint] = {"nx", "ny", "nz"};
  testParam<unsigned int>(nint, int_params, "TestB");
}

template <typename T>
void
EBSDMeshErrorTest::testParam(unsigned int nparam, const char ** param_list, std::string name)
{
  for (unsigned int i = 0; i < nparam; ++i)
  {
    // create a unique name
    std::ostringstream oss;
    oss << name << "_" << i;

    // generate input parameter set
    InputParameters params = validParams<EBSDMesh>();
    params.addPrivateParam("_moose_app", _app);
    params.set<std::string>("_object_name") = oss.str();

    // set a single parameter
    params.set<T>(param_list[i]) = T(1.0);

    // set filename (is a required param but not used in these tests)
    params.set<FileName>("filename") = "DUMMY";

    try
    {
      // construct mesh object
      std::unique_ptr<EBSDMesh> mesh(new EBSDMesh(params));
    }
    catch (const std::exception & e)
    {
      std::string msg(e.what());
      CPPUNIT_ASSERT(
          msg.find("Do not specify mesh geometry information, it is read from the EBSD file.") !=
          std::string::npos);
    }
  }
}
