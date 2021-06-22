//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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

  /// get a function pointer of type T to a function exported from the loaded library
  template <typename T>
  T getFunction(std::string func);

private:
  /// Library handle returned by dlopen
  void * _handle;
};

template <typename T>
T
DynamicLibraryLoader::getFunction(std::string func)
{
#ifdef LIBMESH_HAVE_DLOPEN
  // Snag the function pointers from the library
  void * pointer;
  pointer = dlsym(_handle, func.c_str());
  T func_ptr = *reinterpret_cast<T *>(&pointer);

  // Catch errors
  const char * dlsym_error = dlerror();
  if (dlsym_error)
  {
    dlclose(_handle);
    mooseError("In DynamicLibraryLoader: ", dlsym_error);
  }
  return func_ptr;
#else
  // this can never be reached as the object instantiation would have already failed
  return nullptr;
#endif
}
