//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Console.h"

class OutputObjectTest : public Console
{
public:
  /**
   * Class constructor
   * @param name
   * @param InputParameters
   */
  static InputParameters validParams();

  OutputObjectTest(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~OutputObjectTest();

  void initialSetup();

private:
  MooseEnum _type;
};
