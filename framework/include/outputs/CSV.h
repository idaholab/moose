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
#include "TableOutputter.h"

// Forward declerations
class CSV;

template<>
InputParameters validParams<CSV>();

/**
 * Based class for adding basic filename support to output base class
 *
 * @see Exodus
 */
class CSV :
  public TableOutputter
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
};

#endif /* CSV_H */
