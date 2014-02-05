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

#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include <string>
// MOOSE includes
#include "InputParameters.h"
#include "Restartable.h"

// libMesh
#include "libmesh/parallel.h"

class Postprocessor;

template<>
InputParameters validParams<Postprocessor>();


class Postprocessor
{
public:
  Postprocessor(const std::string & name, InputParameters parameters);

  virtual ~Postprocessor(){ }

  /**
   * This will get called to actually grab the final value the postprocessor has calculated.
   */
  virtual PostprocessorValue getValue() = 0;

  /**
   * Get the postprocessor output type
   * @return postprocessor output type
   */
  Moose::PPSOutputType getOutput() { return _output; }

  /**
   * Get the list of output objects that this class is restricted
   * @return A vector of OutputNames
   */
  std::set<OutputName> getOutputs(){ return std::set<OutputName>(_outputs.begin(), _outputs.end()); }

  /**
   * Returns the name of the Postprocessor.
   */
  std::string PPName() { return _pp_name; }

protected:
  std::string _pp_name;

  /// If and where is the postprocessor output
  Moose::PPSOutputType _output;

  /// Vector of output names
  std::vector<OutputName> _outputs;
};

#endif
