//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

class TestDeclareReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  TestDeclareReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  int & _int; // MooseDocs:producer
  Real & _real;
  std::vector<Real> & _vector;
  std::string & _string;
  Real & _bcast_value;

  std::vector<dof_id_type> _values_to_scatter;
  dof_id_type & _scatter_value;

  std::vector<dof_id_type> _values_to_gather;
  std::vector<dof_id_type> & _gather_value;

  std::vector<dof_id_type> * _distributed_vector;
};

class TestGetReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  TestGetReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  const int & _int; // MooseDocs:consumer
  const int & _int_old;

  const Real & _real;
  const std::vector<Real> & _vector;
  const std::string & _string;
  const Real & _bcast_value;
  const dof_id_type & _scatter_value;
  const std::vector<dof_id_type> & _gather_value;
};

class TestDeclareInitialSetupReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  TestDeclareInitialSetupReporter(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override {}
};

class TestGetReporterDeclaredInInitialSetupReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  TestGetReporterDeclaredInInitialSetupReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  const Real & _value_declared_in_initial_setup;
  Real & _the_value_of_the_reporter;
};

class TestDeclareErrorsReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  TestDeclareErrorsReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override {}
};
