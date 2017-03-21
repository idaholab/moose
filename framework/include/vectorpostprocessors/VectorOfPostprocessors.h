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

#ifndef VECTOR_OF_POSTPROCESSORS_H
#define VECTOR_OF_POSTPROCESSORS_H

#include "GeneralVectorPostprocessor.h"

// Forward Declarations
class VectorOfPostprocessors;

template <>
InputParameters validParams<VectorOfPostprocessors>();

/**
 *  VectorOfPostprocessors is a type of VectorPostprocessor that outputs the
 *  values of an arbitrary user-specified set of postprocessors as a vector in the order specified
 * by the user.
 */

class VectorOfPostprocessors : public GeneralVectorPostprocessor
{
public:
  /**
    * Class constructor
    * @param parameters The input parameters
    */
  VectorOfPostprocessors(const InputParameters & parameters);

  /**
   * Initialize, clears the postprocessor vector
   */
  virtual void initialize() override;

  /**
   * Populates the postprocessor vector of values with the supplied postprocessors
   */
  virtual void execute() override;

protected:
  /// The VectorPostprocessorValue object where the results are stored
  VectorPostprocessorValue & _pp_vec;

  /// The vector of PostprocessorValue objects that are used to get the values of the postprocessors
  std::vector<const PostprocessorValue *> _postprocessor_values;
};

#endif
