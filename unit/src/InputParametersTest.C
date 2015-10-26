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

#include "InputParametersTest.h"

// MOOSE includes
#include "InputParameters.h"

CPPUNIT_TEST_SUITE_REGISTRATION( InputParametersTest );

void
InputParametersTest::checkControlParamPrivateError()
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.addPrivateParam<Real>("private", 1);
    params.declareControllable("private");
    params.checkParams("");
    CPPUNIT_ASSERT( false ); // shouldn't get here
  }
  catch(const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT( msg.find("is a private parameter") != std::string::npos );
  }
}

void
InputParametersTest::checkControlParamTypeError()
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.addParam<PostprocessorName>("pp_name", "make_it_valid", "Some doc");
    params.declareControllable("pp_name");
    params.checkParams("");
    CPPUNIT_ASSERT( false ); // shouldn't get here
  }
  catch(const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT( msg.find("cannot be marked as controllable because its type") != std::string::npos );
  }
}

void
InputParametersTest::checkControlParamValidError()
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.declareControllable("not_valid");
    params.checkParams("");
    CPPUNIT_ASSERT( false ); // shouldn't get here
  }
  catch(const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT( msg.find("The parameter 'not_valid'") != std::string::npos );
  }
}

void
InputParametersTest::checkSuppressedError()
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.suppressParameter<int>("nonexistent");
    CPPUNIT_ASSERT( false ); // shouldn't get here
  }
  catch(const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT( msg.find("Unable to suppress nonexistent parameter") != std::string::npos );
  }
}
