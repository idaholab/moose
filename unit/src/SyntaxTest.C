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

#include "SyntaxTest.h"
#include "Syntax.h"

CPPUNIT_TEST_SUITE_REGISTRATION(SyntaxTest);

void
SyntaxTest::setUp()
{
  _syntax.registerTaskName("first", false);
  _syntax.registerTaskName("second", true);

  // MOOSE object task
  _syntax.registerTaskName("first_mo", "MooseSystem1", false);
  _syntax.registerTaskName("second_mo", "MooseSystem2", true);

  _syntax.registerActionSyntax("SomeAction", "TopBlock");
  _syntax.registerActionSyntax("SomeAction", "TopBlock", "second");
}

void
SyntaxTest::errorChecks()
{
  try
  {
    _syntax.registerTaskName("first", true);

    // Unreachable
    CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    CPPUNIT_ASSERT(msg.find("is already registered") != std::string::npos);
  }

  try
  {
    _syntax.registerTaskName("second_mo", false);

    // Unreachable
    CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    CPPUNIT_ASSERT(msg.find("is already registered") != std::string::npos);
  }

  try
  {
    _syntax.appendTaskName("third", "MooseSystem");

    // Unreachable
    CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    CPPUNIT_ASSERT(msg.find("is not a registered task name") != std::string::npos);
  }

  try
  {
    _syntax.addDependency("forth", "third");

    // Unreachable
    CPPUNIT_ASSERT(false);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());

    CPPUNIT_ASSERT(msg.find("is not a registered task name") != std::string::npos);
  }
}

void
SyntaxTest::testGeneral()
{
  CPPUNIT_ASSERT(_syntax.hasTask("second"));
  CPPUNIT_ASSERT(_syntax.hasTask("third") == false);

  CPPUNIT_ASSERT(_syntax.isActionRequired("first") == false);
  CPPUNIT_ASSERT(_syntax.isActionRequired("second_mo"));

  // TODO: test this
  _syntax.replaceActionSyntax("MooseSystem", "NewBlock", "first");
}

void
SyntaxTest::testDeprecated()
{
  _syntax.deprecateActionSyntax("TopBlock");

  CPPUNIT_ASSERT(_syntax.isDeprecatedSyntax("TopBlock"));
}

void
SyntaxTest::tearDown()
{
}
