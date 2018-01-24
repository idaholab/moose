//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef OUTPUTOBJECTTEST_H
#define OUTPUTOBJECTTEST_H

// MOOSE includes
#include "Console.h"

// Forward declerations
class OutputObjectTest;

template <>
InputParameters validParams<OutputObjectTest>();

/**
 *
 */
class OutputObjectTest : public Console
{
public:
  /**
   * Class constructor
   * @param name
   * @param InputParameters
   */
  OutputObjectTest(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~OutputObjectTest();

  void initialSetup();

private:
  MooseEnum _type;
};

#endif // OUTPUTOBJECTTEST_H
