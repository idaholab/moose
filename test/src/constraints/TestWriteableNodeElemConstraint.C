//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "TestWriteableNodeElemConstraint.h"

registerMooseObject("MooseTestApp", TestWriteableNodeElemConstraint);

InputParameters
TestWriteableNodeElemConstraint::validParams()
{
  InputParameters params = EqualValueEmbeddedConstraint::validParams();
  params.addClassDescription("tests writeable variables for NodeElemConstraints.");
  params.addCoupledVar("output_writeable_var1",
                       "Writeable output variable that will be written to.");
  params.addCoupledVar("output_writeable_var2",
                       "Writeable output variable that will remain a nullptr.");
  return params;
}

TestWriteableNodeElemConstraint::TestWriteableNodeElemConstraint(const InputParameters & parameters)
  : EqualValueEmbeddedConstraint(parameters)
{
  if (this->isParamValid("output_writeable_var1"))
    _output_writeable_var1 = &(writableVariable("output_writeable_var1"));
  if (this->isParamValid("output_writeable_var2"))
    paramError("output_writeable_var2", "this variable checks that output is optional.");
}

void
TestWriteableNodeElemConstraint::reinitConstraint()
{
  EqualValueEmbeddedConstraint::reinitConstraint();
  const Node * node = _current_node;
  if (_output_writeable_var1)
    _output_writeable_var1->setNodalValue(node->id());
}
