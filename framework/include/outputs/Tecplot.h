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

#ifndef TECPLOT_H
#define TECPLOT_H

// MOOSE includes
#include "BasicOutput.h"
#include "OversampleOutput.h"

// Forward declarations
class Tecplot;

template <>
InputParameters validParams<Tecplot>();

/**
 * Class for output data to the TecplotII format
 */
class Tecplot : public BasicOutput<OversampleOutput>
{
public:
  /**
   * Class constructor
   */
  Tecplot(const InputParameters & parameters);

protected:
  /**
   * Overload the Output::output method, this is required for Tecplot
   * output due to the method utilized for outputing single/global parameters
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * Returns the current filename, this method handles adding the timestep suffix
   * @return A string containing the current filename to be written
   */
  virtual std::string filename() override;

private:
  /// Flag for binary output
  bool _binary;

  /// Flag for turning on appending to ASCII files
  bool _ascii_append;

  /// True if this is the first time the file has been written to,
  /// gets set to false after the first call to output().  If the user
  /// has set _ascii_append but _first_time==true, we won't actually
  /// append.  This prevents old data files in a directory from being
  /// appended to.  Declared as a reference so it can be restartable
  /// data, that way if we restart, we don't think it's the first time
  /// again.
  bool & _first_time;
};

#endif /* TECPLOT_H */
