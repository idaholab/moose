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

#include <exception>

/**
 * Provides a way for users to bail out of the current solve.
 */
class MooseException : public std::exception
{
public:
  /**
   * @param message The message to display
   */
  MooseException(std::string message) : _message(message) {}

  /**
   * For some reason, on GCC 4.6.3, I get 'error: looser throw
   * specifier' when deriving from std::exception unless I declare
   * that the destructor *won't* throw by adding the throw()
   * specification.  Clang doesn't seem to care about this line of
   * code.
   */
  ~MooseException() throw() {}

  /**
   * Get out the error message.
   *
   * Satisfies the interface of std::exception
   */
  virtual const char * what() const throw() { return _message.c_str(); }

protected:
  std::string _message;
};

#endif /* MOOSEEXCEPTION_H */
