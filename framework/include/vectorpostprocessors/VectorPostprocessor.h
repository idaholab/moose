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

#ifndef VECTORPOSTPROCESSOR_H
#define VECTORPOSTPROCESSOR_H

// MOOSE includes
#include "InputParameters.h"
#include "MooseTypes.h"
#include "OutputInterface.h"

// libMesh
#include "libmesh/parallel.h"

// Forward declarations
class SamplerBase;
class VectorPostprocessor;
class VectorPostprocessorData;
class FEProblemBase;

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
