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
 * This Macro is used to generate std::string derived types useful for
 * strong type checking and special handling in the GUI.  It does not
 * extend std::string in any way so it is generally "safe"
 */
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

// Instantiate new Types

/// This type is for expected filenames, it can be used to trigger open file dialogs in the GUI
DerivativeStringClass(FileName);
/// This type is similar to "FileName", but is used to further filter file dialogs on known file mesh types
DerivativeStringClass(MeshFileName);

/// This type is used for objects that expect nonlinear variable names (i.e. Kernels, BCs)
DerivativeStringClass(NonlinearVariableName);
/// This type is used for objects that expect Auxiliary variable names (i.e. AuxKernels, AuxBCs)
DerivativeStringClass(AuxVariableName);
/// This type is used for objects that expect either Nonlinear or Auxiliary Variables such as postprocessors
DerivativeStringClass(VariableName);

/// This type is used for objects that expect Boundary Names/Ids read from or generated on the current mesh
DerivativeStringClass(BoundaryName);
/// This type is similar to BoundaryName but is used for "blocks" or subdomains in the current mesh
DerivativeStringClass(SubdomainName);

/// This type is used for objects that expect Postprocessor objects
DerivativeStringClass(PostprocessorName);

/// This type is used for objects that expect Moose Function objects
DerivativeStringClass(FunctionName);

/// This type is used for objects that expect "UserObject" names
DerivativeStringClass(UserObjectName);

#endif // FILENAME_H
