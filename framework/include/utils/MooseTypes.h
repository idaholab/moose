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

#define DerivativeStringClass(TheName)                                  \
  class TheName : public std::string                                    \
  {                                                                     \
  public:                                                               \
    TheName(): std::string() {}                                         \
    TheName(const std::string& str): std::string(str) {}                \
    TheName(const std::string& str, size_t pos, size_t n = npos):       \
      std::string(str, pos, n) {}                                       \
    TheName(const char * s, size_t n): std::string(s,n) {}              \
    TheName(const char * s): std::string(s) {}                          \
    TheName(size_t n, char c): std::string(n, c) {}                     \
  } /* No semicolon here because this is a macro */

// Create Classes
DerivativeStringClass(FileName);
DerivativeStringClass(VariableName);
DerivativeStringClass(NonlinearVariableName);
DerivativeStringClass(AuxVariableName);

#endif // FILENAME_H
