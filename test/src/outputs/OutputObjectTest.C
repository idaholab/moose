//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OutputObjectTest.h"
#include "MooseApp.h"
#include "Exodus.h"

registerMooseObject("MooseTestApp", OutputObjectTest);

InputParameters
OutputObjectTest::validParams()
{
  MooseEnum type("getOutput getOutputs-names getOutputs getOutputNames");
  InputParameters params = Console::validParams();
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
