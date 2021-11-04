//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"
#include "libmesh/libmesh_config.h"
#include <string>

#ifdef LIBMESH_HAVE_DLOPEN
#include <dlfcn.h>
#endif

/**
 * Wrapper class to facilitate loading and lifetime management of dynamic libraries and obtaining
 * pointers to exported functions.
 */
class DynamicLibraryLoader
{
public:
  DynamicLibraryLoader(const std::string & library_file);
  ~DynamicLibraryLoader();

  /**
   * Get a function/data pointer of type T to a function exported from the loaded library.
   * @param hard_fail Set this to false to return a nullptr if a symbol was not found.
   */
  template <typename T>
  T getFunction(std::string func, bool hard_fail = true);

private:
  /// Library handle returned by dlopen
  void * _handle;

  /// Library file name
  const std::string _library_file;
};

template <typename T>
T
DynamicLibraryLoader::getFunction(std::string func, bool hard_fail)
{
#ifdef LIBMESH_HAVE_DLOPEN
  // clear error
  dlerror();

  // Snag the function pointers from the library
  void * pointer = dlsym(_handle, func.c_str());

  // Catch errors
  const char * dlsym_error = dlerror();
  if (dlsym_error && hard_fail)
  {
    dlclose(_handle);
    mooseError("DynamicLibraryLoader error: Unable to find symbol '",
               func,
               "' in library '",
               _library_file,
               "'. ",
               std::string(dlsym_error));
  }

  return *reinterpret_cast<T *>(&pointer);
#else
  // this can never be reached as the object instantiation would have already failed
  return nullptr;
#endif
}
