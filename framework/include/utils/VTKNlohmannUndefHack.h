#pragma once

// If VTK is built without an external nlohmann, then it assumes it
// will never be compiled against another nlohmann, and it includes
// its own copy but modified with macro tricks.  We have probably
// already included nlohmann headers, which we didn't tamper with
// because OF COURSE NOT, but now we need to take care not to let the
// include guards prevent them from including their copy with their
// different namespace.
#ifndef MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS
#define MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS 0
// Detect if VTK built with external nlohmann
#ifdef __has_include
#if __has_include("vtk_nlohmannjson.h")
#include "vtk_nlohmannjson.h"
#if !VTK_MODULE_USE_EXTERNAL_vtknlohmannjson
#undef MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS
#define MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS 1
#endif // !VTK_MODULE_USE_EXTERNAL_vtknlohmannjson
#endif // __has_include("vtk_nlohmannjson.h")
#else  // __has_include
#error "Could not auto-detect whether VTK built with external nlohmann json. \
Define MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS=1 if built with vendored nlohmann \
, otherwise define MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS=0"
#endif // __has_include
#endif // MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS
