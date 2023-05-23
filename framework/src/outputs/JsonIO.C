//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JsonIO.h"
#include "MooseApp.h"
#include "MooseRevision.h"
#include "SystemInfo.h"

#include "libmesh/libmesh_config.h"

// MooseDocs:to_json_start
void
to_json(nlohmann::json & json, const MooseApp & app)
{
  if (!app.getSystemInfo())
    return;

  json["app_name"] = app.name();
  json["current_time"] = app.getSystemInfo()->getTimeStamp();
  json["executable"] = app.getSystemInfo()->getExecutable();
  json["executable_time"] = app.getSystemInfo()->getExecutableTimeStamp(json["executable"]);

  json["moose_version"] = MOOSE_REVISION;
  json["libmesh_version"] = LIBMESH_BUILD_VERSION;
#ifdef LIBMESH_DETECTED_PETSC_VERSION_MAJOR
  json["petsc_version"] = std::to_string(LIBMESH_DETECTED_PETSC_VERSION_MAJOR) + "." +
                          std::to_string(LIBMESH_DETECTED_PETSC_VERSION_MINOR) + "." +
                          std::to_string(LIBMESH_DETECTED_PETSC_VERSION_SUBMINOR);
#endif
#ifdef LIBMESH_DETECTED_SLEPC_VERSION_MAJOR
  json["slepc_version"] = std::to_string(LIBMESH_DETECTED_SLEPC_VERSION_MAJOR) + "." +
                          std::to_string(LIBMESH_DETECTED_SLEPC_VERSION_MINOR) + "." +
                          std::to_string(LIBMESH_DETECTED_SLEPC_VERSION_SUBMINOR);
#endif
}
// MooseDocs:to_json_end

namespace libMesh
{
void
to_json(nlohmann::json & json, const libMesh::Point & p)
{
  json["x"] = p(0);
  json["y"] = p(1);
  json["z"] = p(2);
}

void
to_json(nlohmann::json & json, const DenseVector<Real> & vector)
{
  nlohmann::to_json(json, vector.get_values());
}
}
