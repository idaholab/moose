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

#include "OutputObjectTest.h"
#include "MooseApp.h"
#include "Exodus.h"

template <>
InputParameters
validParams<OutputObjectTest>()
{
  MooseEnum type("getOutput getOutputs-names getOutputs getOutputNames");
  InputParameters params = validParams<Console>();
  params.addRequiredParam<MooseEnum>("test_type", type, "The type of test to execute");
  return params;
}

OutputObjectTest::OutputObjectTest(const InputParameters & parameters)
  : Console(parameters), _type(getParam<MooseEnum>("test_type"))
{
}

OutputObjectTest::~OutputObjectTest() {}

void
OutputObjectTest::initialSetup()
{
  Console::initialSetup();

  if (_type == "getOutput")
  {
    OutputObjectTest * ptr = _app.getOutputWarehouse().getOutput<OutputObjectTest>(name());
    if (ptr == this)
      mooseError("getOutput test passed");
  }

  else if (_type == "getOutputs")
  {
    std::vector<Exodus *> ptrs = _app.getOutputWarehouse().getOutputs<Exodus>();
    if (ptrs.size() == 2 && ptrs[0]->name().compare("exodus") == 0 &&
        ptrs[1]->name().compare("exodus2") == 0)
      mooseError("getOutputs test passed");
  }

  else if (_type == "getOutputs-names")
  {
    std::vector<OutputName> names;
    names.push_back("exodus2");
    names.push_back("exodus");
    std::vector<Exodus *> ptrs = _app.getOutputWarehouse().getOutputs<Exodus>(names);
    if (ptrs.size() == 2 && ptrs[0]->name().compare("exodus2") == 0 &&
        ptrs[1]->name().compare("exodus") == 0)
      mooseError("getOutputs-names test passed");
  }

  else if (_type == "getOutputNames")
  {
    std::vector<OutputName> names = _app.getOutputWarehouse().getOutputNames<Exodus>();
    if (names.size() == 2 && names[0].compare("exodus") == 0 && names[1].compare("exodus2") == 0)
      mooseError("getOutputsNames test passed");
  }

  else
    mooseError("You must specify a 'test_type'");
}
