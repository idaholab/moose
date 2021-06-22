//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DynamicLibraryLoader.h"

DynamicLibraryLoader::DynamicLibraryLoader(const std::string & library_file)
{
#ifdef LIBMESH_HAVE_DLOPEN
  _handle = dlopen(library_file.c_str(), RTLD_LAZY);
  if (!_handle)
    mooseError("Failed to load libary '", library_file, "' in DynamicLibraryLoader: ", dlerror());

  dlerror();
#else
  // support for the POSIX function 'dlopen' to dynamically load libraries is missing
  mooseError("Dynamic library loading is not supported on this operating system.");
#endif
}

DynamicLibraryLoader::~DynamicLibraryLoader()
{
#ifdef LIBMESH_HAVE_DLOPEN
  dlclose(_handle);
#endif
}
