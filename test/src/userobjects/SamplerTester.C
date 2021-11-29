//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SamplerTester.h"

registerMooseObject("MooseTestApp", SamplerTester);

InputParameters
SamplerTester::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<SamplerName>("sampler", "The sampler to test.");

  MooseEnum test_type(
      "mpi thread base_global_vs_local rand_global_vs_local rand_global_vs_next getGlobalSamples "
      "getLocalSamples getNextLocalRow");
  params.addParam<MooseEnum>("test_type", test_type, "The type of test to perform.");
  params.set<std::vector<OutputName>>("outputs") = {"none"};
  params.suppressParameter<std::vector<OutputName>>("outputs");
  return params;
}

SamplerTester::SamplerTester(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _sampler(getSampler("sampler")),
    _test_type(getParam<MooseEnum>("test_type"))
{
}

Real
SamplerTester::getValue()
{
  return 0.0;
}

void
SamplerTester::execute()
{
  if (_test_type == "getGlobalSamples")
    _samples = _sampler.getGlobalSamples();

  if (_test_type == "getLocalSamples")
    _samples = _sampler.getLocalSamples();

  if (_test_type == "getNextLocalRow")
    for (dof_id_type i = _sampler.getLocalRowBegin(); i < _sampler.getLocalRowEnd(); ++i)
      std::vector<Real> row = _sampler.getNextLocalRow();

  if (_test_type == "rand_global_vs_next")
  {
    mooseAssert(n_processors() == 1, "This test only works on one processor.");

    // Get the full set of samples
    DenseMatrix<Real> global = _sampler.getGlobalSamples();

    // Iterate through some
    for (dof_id_type i = _sampler.getLocalRowBegin(); i < 7; ++i)
    {
      std::vector<Real> row = _sampler.getNextLocalRow();
      for (unsigned int j = 0; j < 8; j++)
      {
        assertEqual(row[j], global(i, j));
      }
    }

    // Get the samples again
    DenseMatrix<Real> local = _sampler.getLocalSamples();

    // Continue iteration
    for (dof_id_type i = 7; i < _sampler.getLocalRowEnd(); ++i)
    {
      std::vector<Real> row = _sampler.getNextLocalRow();
      for (unsigned int j = 0; j < 8; j++)
      {
        assertEqual(row[j], global(i, j));
      }
    }
  }

  if (_test_type == "rand_global_vs_local")
  {
    DenseMatrix<Real> global = _sampler.getGlobalSamples();
    DenseMatrix<Real> local = _sampler.getLocalSamples();
    if (n_processors() == 1)
    {
      assertEqual(global.m(), 14);
      assertEqual(global.n(), 8);
      assertEqual(local.m(), 14);
      assertEqual(local.n(), 8);
      assertEqual(global, local);

      for (dof_id_type i = _sampler.getLocalRowBegin(); i < _sampler.getLocalRowEnd(); ++i)
      {
        std::vector<Real> row = _sampler.getNextLocalRow();
        for (unsigned int j = 0; j < 8; j++)
        {
          assertEqual(row[j], global(i, j));
        }
      }
    }

    else if (n_processors() == 2)
    {
      assertEqual(global.m(), 14);
      assertEqual(global.n(), 8);
      if (processor_id() == 0)
      {
        assertEqual(local.m(), 7);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), global(0, i));
          assertEqual(local(1, i), global(1, i));
          assertEqual(local(2, i), global(2, i));
          assertEqual(local(3, i), global(3, i));
          assertEqual(local(4, i), global(4, i));
          assertEqual(local(5, i), global(5, i));
          assertEqual(local(6, i), global(6, i));
        }

        for (dof_id_type i = _sampler.getLocalRowBegin(); i < _sampler.getLocalRowEnd(); ++i)
        {
          std::vector<Real> row = _sampler.getNextLocalRow();
          for (unsigned int j = 0; j < 8; j++)
          {
            assertEqual(row[j], global(i, j));
          }
        }
      }

      else if (processor_id() == 1)
      {
        assertEqual(local.m(), 7);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), global(7, i));
          assertEqual(local(1, i), global(8, i));
          assertEqual(local(2, i), global(9, i));
          assertEqual(local(3, i), global(10, i));
          assertEqual(local(4, i), global(11, i));
          assertEqual(local(5, i), global(12, i));
          assertEqual(local(6, i), global(13, i));
        }

        for (dof_id_type i = _sampler.getLocalRowBegin(); i < _sampler.getLocalRowEnd(); ++i)
        {
          std::vector<Real> row = _sampler.getNextLocalRow();
          for (unsigned int j = 0; j < 8; j++)
          {
            assertEqual(row[j], global(i, j));
          }
        }
      }
    }

    else if (n_processors() == 3)
    {
      assertEqual(global.m(), 14);
      assertEqual(global.n(), 8);
      if (processor_id() == 0)
      {
        assertEqual(local.m(), 5);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), global(0, i));
          assertEqual(local(1, i), global(1, i));
          assertEqual(local(2, i), global(2, i));
          assertEqual(local(3, i), global(3, i));
          assertEqual(local(4, i), global(4, i));
        }

        for (dof_id_type i = _sampler.getLocalRowBegin(); i < _sampler.getLocalRowEnd(); ++i)
        {
          std::vector<Real> row = _sampler.getNextLocalRow();
          for (unsigned int j = 0; j < 8; j++)
          {
            assertEqual(row[j], global(i, j));
          }
        }
      }

      else if (processor_id() == 1)
      {
        assertEqual(local.m(), 5);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), global(5, i));
          assertEqual(local(1, i), global(6, i));
          assertEqual(local(2, i), global(7, i));
          assertEqual(local(3, i), global(8, i));
          assertEqual(local(4, i), global(9, i));
        }

        for (dof_id_type i = _sampler.getLocalRowBegin(); i < _sampler.getLocalRowEnd(); ++i)
        {
          std::vector<Real> row = _sampler.getNextLocalRow();
          for (unsigned int j = 0; j < 8; j++)
          {
            assertEqual(row[j], global(i, j));
          }
        }
      }

      else if (processor_id() == 2)
      {
        assertEqual(local.m(), 4);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), global(10, i));
          assertEqual(local(1, i), global(11, i));
          assertEqual(local(2, i), global(12, i));
          assertEqual(local(3, i), global(13, i));
        }

        for (dof_id_type i = _sampler.getLocalRowBegin(); i < _sampler.getLocalRowEnd(); ++i)
        {
          std::vector<Real> row = _sampler.getNextLocalRow();
          for (unsigned int j = 0; j < 8; j++)
          {
            assertEqual(row[j], global(i, j));
          }
        }
      }
    }
  }

  if (_test_type == "base_global_vs_local")
  {
    DenseMatrix<Real> global = _sampler.getGlobalSamples();
    DenseMatrix<Real> local = _sampler.getLocalSamples();

    if (n_processors() == 1)
    {
      assertEqual(global.m(), 14);
      assertEqual(global.n(), 8);
      assertEqual(local.m(), 14);
      assertEqual(local.n(), 8);
      assertEqual(global, local);
    }

    else if (n_processors() == 2)
    {
      assertEqual(global.m(), 14);
      assertEqual(global.n(), 8);
      if (processor_id() == 0)
      {
        assertEqual(local.m(), 7);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), 10 + i);
          assertEqual(local(1, i), 20 + i);
          assertEqual(local(2, i), 30 + i);
          assertEqual(local(3, i), 40 + i);
          assertEqual(local(4, i), 50 + i);
          assertEqual(local(5, i), 60 + i);
          assertEqual(local(6, i), 70 + i);
        }
      }

      else if (processor_id() == 1)
      {
        assertEqual(local.m(), 7);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), 80 + i);
          assertEqual(local(1, i), 90 + i);
          assertEqual(local(2, i), 100 + i);
          assertEqual(local(3, i), 110 + i);
          assertEqual(local(4, i), 120 + i);
          assertEqual(local(5, i), 130 + i);
          assertEqual(local(6, i), 140 + i);
        }
      }
    }

    else if (n_processors() == 3)
    {
      assertEqual(global.m(), 14);
      assertEqual(global.n(), 8);
      if (processor_id() == 0)
      {
        assertEqual(local.m(), 5);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), 10 + i);
          assertEqual(local(1, i), 20 + i);
          assertEqual(local(2, i), 30 + i);
          assertEqual(local(3, i), 40 + i);
          assertEqual(local(4, i), 50 + i);
        }
      }

      else if (processor_id() == 1)
      {
        assertEqual(local.m(), 5);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), 60 + i);
          assertEqual(local(1, i), 70 + i);
          assertEqual(local(2, i), 80 + i);
          assertEqual(local(3, i), 90 + i);
          assertEqual(local(4, i), 100 + i);
        }
      }

      else if (processor_id() == 2)
      {
        assertEqual(local.m(), 4);
        assertEqual(local.n(), 8);
        for (unsigned int i = 0; i < 8; ++i)
        {
          assertEqual(local(0, i), 110 + i);
          assertEqual(local(1, i), 120 + i);
          assertEqual(local(2, i), 130 + i);
          assertEqual(local(3, i), 140 + i);
        }
      }
    }
  }
}

void
SamplerTester::initialize()
{
  _samples.resize(0, 0);
}

void
SamplerTester::finalize()
{
  if (_communicator.size() > 1 && _test_type == "mpi")
  {
    std::size_t vec_size;
    std::vector<Real> samples;
    if (_communicator.rank() == 0)
    {
      samples = _sampler.getGlobalSamples().get_values();
      vec_size = samples.size();
    }

    _communicator.broadcast(vec_size);
    samples.resize(vec_size);
    _communicator.broadcast(samples);

    if (_sampler.getGlobalSamples().get_values() != samples)
      mooseError("The sample generation is not working correctly with MPI.");
  }
}

void
SamplerTester::threadJoin(const UserObject & uo)
{
  if (_test_type == "thread")
  {
    const SamplerTester & other = static_cast<const SamplerTester &>(uo);
    if (_sampler.getGlobalSamples() != other._sampler.getGlobalSamples())
      mooseError("The sample generation is not working correctly with threads.");
  }
}

void
SamplerTester::assertEqual(const Real & value, const Real & gold)
{
  if (value != gold)
    mooseError(value, " != ", gold);
}

void
SamplerTester::assertEqual(const DenseMatrix<Real> & value, const DenseMatrix<Real> & gold)
{
  if (value != gold)
    mooseError(value, " != ", gold);
}
