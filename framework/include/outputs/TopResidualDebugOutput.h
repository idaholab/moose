//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TOPRESIDUALEBUGOUTPUT_H
#define TOPRESIDUALEBUGOUTPUT_H

// MOOSE includes
#include "PetscOutput.h"

#include "libmesh/system.h"

// Forward declerations
class TopResidualDebugOutput;

template <>
InputParameters validParams<TopResidualDebugOutput>();

/**
 * A structure for storing data related to top residuals
 *  @see TopResidualDebugOutput::printTopResiduals()
 */
struct TopResidualDebugOutputTopResidualData
{
  unsigned int _var;
  dof_id_type _nd;
  Real _residual;
  bool _is_scalar;

  TopResidualDebugOutputTopResidualData() : _var(0), _nd(0), _residual(0.), _is_scalar(false) {}

  TopResidualDebugOutputTopResidualData(unsigned int var,
                                        dof_id_type nd,
                                        Real residual,
                                        bool is_scalar = false)
    : _var(var), _nd(nd), _residual(residual), _is_scalar(is_scalar)
  {
  }
};

/**
 * A class for producing various debug related outputs
 *
 * This class may be used from inside the [Outputs] block or via the [Debug] block (preferred)
 */
class TopResidualDebugOutput : public PetscOutput
{
public:
  /**
   * Class constructor
   * @param parameters Object input parameters
   */
  TopResidualDebugOutput(const InputParameters & parameters);

protected:
  /**
   * Perform the debugging output
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * Prints the n top residuals for the variables in the system
   * @param residual A reference to the residual vector
   * @param n The number of residuals to print
   */
  void printTopResiduals(const NumericVector<Number> & residual, unsigned int n);

  /**
   * Method for sorting the residuals data from TopResidualDebugOutputTopResidualData structs
   * @see printTopResiduals
   */
  static bool sortTopResidualData(TopResidualDebugOutputTopResidualData i,
                                  TopResidualDebugOutputTopResidualData j)
  {
    return (fabs(i._residual) > fabs(j._residual));
  }

  /// Number of residuals to display
  unsigned int _num_residuals;

  /// Reference to libMesh system
  System & _sys;
};

#endif // TOPRESIDUALDEBUGOUTPUT_H
