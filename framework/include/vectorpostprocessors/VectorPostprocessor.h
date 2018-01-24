//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORPOSTPROCESSOR_H
#define VECTORPOSTPROCESSOR_H

// MOOSE includes
#include "MooseTypes.h"
#include "OutputInterface.h"

// libMesh
#include "libmesh/parallel.h"

// Forward declarations
class FEProblemBase;
class InputParameters;
class SamplerBase;
class VectorPostprocessor;
class VectorPostprocessorData;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<VectorPostprocessor>();

/**
 * Base class for Postprocessors that produce a vector of values.
 */
class VectorPostprocessor : public OutputInterface
{
public:
  VectorPostprocessor(const InputParameters & parameters);

  virtual ~VectorPostprocessor() = default;

  /**
   * This will get called to actually grab the final value the VectorPostprocessor has calculated.
   */
  virtual VectorPostprocessorValue & getVector(const std::string & vector_name);

  /**
   * Returns the name of the VectorPostprocessor.
   */
  std::string PPName() { return _vpp_name; }

protected:
  /**
   * Register a new vector to fill up.
   */
  VectorPostprocessorValue & declareVector(const std::string & vector_name);

  /// The name of the VectorPostprocessor
  std::string _vpp_name;

  /// Pointer to FEProblemBase
  FEProblemBase * _vpp_fe_problem;

  friend class SamplerBase;

private:
  THREAD_ID _vpp_tid;

  std::map<std::string, VectorPostprocessorValue> _thread_local_vectors;
};

#endif
