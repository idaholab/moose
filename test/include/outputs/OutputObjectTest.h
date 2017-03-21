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
