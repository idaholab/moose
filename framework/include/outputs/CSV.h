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

#ifndef CSV_H
#define CSV_H

// MOOSE includes
#include "TableOutput.h"

// Forward declarations
class CSV;

template<>
InputParameters validParams<CSV>();

/**
 * Based class for adding basic filename support to output base class
 *
 * @see Exodus
 */
class CSV :
  public TableOutput
{
public:

  /**
   * Class constructor
   *
   * The constructor performs all of the necessary initialization of the various
   * output lists required for the various output types.
   *
   * @see initAvailable init separate
   */
  CSV(const std::string & name, InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~CSV();

  /**
   * Output the table to a *.csv file
   */
  virtual void output();

  /**
   * The filename for the output file
   * @return A string of output file including the extension
   */
  virtual std::string filename();

  /**
   * Setup the CSV output
   * If restarting and the append_restart flag is false, then the output data is cleared here
   */
  void initialSetup();

private:

  /// Flag for aligning data in .csv file
  bool _align;
};

#endif /* CSV_H */
