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

#ifndef GNUPLOT_H
#define GNUPLOT_H

// MOOSE includes
#include "TableOutput.h"

// Forward declarations
class Gnuplot;

template <>
InputParameters validParams<Gnuplot>();

/**
 * Based class for adding basic filename support to output base class
 *
 * @see Exodus
 */
class Gnuplot : public TableOutput
{
public:
  /**
   * Class constructor
   *
   * The constructor performs all of the necessary initialization of the various
   * output lists required for the various output types.
   *
   * @see initAvailable init seperate
   */
  Gnuplot(const InputParameters & parameters);

  /**
   * Output the table to a *.csv file
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * The filename for the output file
   * @return A string of output file including the extension
   */
  virtual std::string filename() override;

private:
  /// Desired file extension
  std::string _extension;
};

#endif /* GNUPLOT_H */
