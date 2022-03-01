//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "PetscOutput.h"

#include "libmesh/system.h"

/**
 * A structure for storing data related to top residuals
 *  @see TopResidualDebugOutput::printTopResiduals()
 */
struct TopResidualDebugOutputTopResidualData
{
  unsigned int _var;
  std::set<SubdomainID> _subdomain_ids;
  dof_id_type _id;
  Point _point;
  Real _residual;
  bool _is_scalar;
  bool _is_nodal;

  TopResidualDebugOutputTopResidualData()
    : _var(0),
      _subdomain_ids(),
      _id(0),
      _point(Point()),
      _residual(0.),
      _is_scalar(false),
      _is_nodal(true)
  {
  }

  TopResidualDebugOutputTopResidualData(unsigned int var,
                                        std::set<SubdomainID> subdomain_ids,
                                        dof_id_type id,
                                        Point point,
                                        Real residual,
                                        bool is_scalar = false,
                                        bool is_nodal = true)
    : _var(var),
      _subdomain_ids(subdomain_ids),
      _id(id),
      _point(point),
      _residual(residual),
      _is_scalar(is_scalar),
      _is_nodal(is_nodal)
  {
  }
};

/**
 * A class for producing various debug related outputs
 *
 * This currently considers the following degrees of freedom:
 * \li first component of all nodal variables
 * \li first component of all elemental variables
 * \li all scalar variables
 *
 * This class may be used from inside the [Outputs] block or via the [Debug] block (preferred)
 */
class TopResidualDebugOutput : public PetscOutput
{
public:
  static InputParameters validParams();

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
