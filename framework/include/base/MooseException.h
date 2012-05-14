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

#ifndef MOOSEEXCEPTION_H
#define MOOSEEXCEPTION_H

/**
 * Base class for any MOOSE-exception.
 *
 * Inherit from this class and adjust to your likings.
 */
class MooseException
{
public:
  MooseException() throw();
  virtual ~MooseException() throw();

  /**
   * Clone this exception.
   * @return The cloned instance of this exception
   */
  virtual MooseException * clone();

  /**
   * Get the description of this exception
   * @return The description of this exception
   */
  virtual const char * what() const throw();
};

#endif /* MOOSEEXCEPTION_H */
