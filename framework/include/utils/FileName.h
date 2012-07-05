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

#ifndef FILENAME_H
#define FILENAME_H

#include <string>

/**
 * Proxy class so that we get the right type for file name parameters coming from the input file.
 */
class FileName : public std::string
{
public:
  /**
   * Wrapping all std::string constructors.
   */
  FileName(): std::string() {}
  FileName(const std::string& str): std::string(str) {}
  FileName(const std::string& str, size_t pos, size_t n = npos): std::string(str, pos, n) {}
  FileName(const char * s, size_t n): std::string(s,n) {}
  FileName(const char * s): std::string(s) {}
  FileName(size_t n, char c): std::string(n, c) {}
};

#endif // FILENAME_H
