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

#ifndef TESTCSVREADER_H
#define TESTCSVREADER_H

#include "GeneralUserObject.h"

// Forward Declarations
class TestCSVReader;

template <>
InputParameters validParams<TestCSVReader>();

/**
 * Test class to make certain that CSV data is broadcast correctly.
 */
class TestCSVReader : public GeneralUserObject
{
public:
  TestCSVReader(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  const VectorPostprocessorValue & _vpp_data;
  const processor_id_type & _rank;
  const std::vector<double> & _gold;
};

#endif
