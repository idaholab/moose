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

#include "Conversion.h"
#include "MooseTypes.h"
#include "MooseError.h"
#include <map>
#include <algorithm>
#include "string_to_enum.h"

namespace Moose {

  std::map<std::string, TimeSteppingScheme> timesteppingscheme_type_to_enum;
  std::map<std::string, ExecFlagType> execstore_type_to_enum;
  std::map<std::string, QuadratureType> quadrature_type_to_enum;
  std::map<std::string, CoordinateSystemType> coordinate_system_type_to_enum;
  std::map<std::string, PPSOutputType> pps_output_type_to_enum;

  void initTimeSteppingMap()
  {
    if (timesteppingscheme_type_to_enum.empty())
    {
      timesteppingscheme_type_to_enum["BACKWARD-EULER"] = IMPLICIT_EULER;
      timesteppingscheme_type_to_enum["IMPLICIT-EULER"] = IMPLICIT_EULER;
      timesteppingscheme_type_to_enum["EXPLICIT-EULER"] = EXPLICIT_EULER;
      timesteppingscheme_type_to_enum["CRANK-NICOLSON"] = CRANK_NICOLSON;
      timesteppingscheme_type_to_enum["BDF2"]           = BDF2;
      timesteppingscheme_type_to_enum["PETSC"]          = PETSC_TS;
    }
  }

  void initExecStoreType()
  {
    if (execstore_type_to_enum.empty())
    {
      execstore_type_to_enum["INITIAL"]  = EXEC_INITIAL;
      execstore_type_to_enum["RESIDUAL"] = EXEC_RESIDUAL;
      execstore_type_to_enum["JACOBIAN"] = EXEC_JACOBIAN;
      execstore_type_to_enum["TIMESTEP"] = EXEC_TIMESTEP;
      execstore_type_to_enum["TIMESTEP_BEGIN"] = EXEC_TIMESTEP_BEGIN;
      execstore_type_to_enum["CUSTOM"] = EXEC_CUSTOM;
    }
  }

  void initQuadratureType()
  {
    if (quadrature_type_to_enum.empty())
    {
      quadrature_type_to_enum["CLOUGH"]   = QCLOUGH;
      quadrature_type_to_enum["CONICAL"]  = QCONICAL;
      quadrature_type_to_enum["GAUSS"]    = QGAUSS;
      quadrature_type_to_enum["GRID"]     = QGRID;
      quadrature_type_to_enum["MONOMIAL"] = QMONOMIAL;
      quadrature_type_to_enum["SIMPSON"]  = QSIMPSON;
      quadrature_type_to_enum["TRAP"]     = QTRAP;
    }
  }

  void initCoordinateSystemType()
  {
    if (coordinate_system_type_to_enum.empty())
    {
      coordinate_system_type_to_enum["XYZ"] = COORD_XYZ;
      coordinate_system_type_to_enum["RZ"]  = COORD_RZ;
      coordinate_system_type_to_enum["RSPHERICAL"]  = COORD_RSPHERICAL;
    }
  }

  void initPPSOutputType()
  {
    if (pps_output_type_to_enum.empty())
    {
      pps_output_type_to_enum["NONE"]   = PPS_OUTPUT_NONE;
      pps_output_type_to_enum["AUTO"]   = PPS_OUTPUT_AUTO;
      pps_output_type_to_enum["SCREEN"] = PPS_OUTPUT_SCREEN;
      pps_output_type_to_enum["FILE"]   = PPS_OUTPUT_FILE;
      pps_output_type_to_enum["BOTH"]   = PPS_OUTPUT_BOTH;
    }
  }

  template<>
  TimeSteppingScheme stringToEnum(const std::string & s)
  {
    initTimeSteppingMap();

    std::string upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (!timesteppingscheme_type_to_enum.count(upper))
      mooseError("Unknown time stepping scheme");

    return timesteppingscheme_type_to_enum[upper];
  }

  template<>
  ExecFlagType stringToEnum(const std::string & s)
  {
    initExecStoreType();

    std::string upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (!execstore_type_to_enum.count(upper))
      mooseError("Unknown execution flag");

    return execstore_type_to_enum[upper];
  }

  template<>
  QuadratureType stringToEnum<QuadratureType>(const std::string & s)
  {
    initQuadratureType();

    std::string upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (!quadrature_type_to_enum.count(upper))
      mooseError("Unknown quadrature type");

    return quadrature_type_to_enum[upper];

  }

  template<>
  Order stringToEnum<Order>(const std::string & s)
  {
    std::string upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (upper.compare("AUTO") == 0)
      return INVALID_ORDER;
    else
      return Utility::string_to_enum<Order>(upper);
  }

  template<>
  CoordinateSystemType stringToEnum<CoordinateSystemType>(const std::string & s)
  {
    initCoordinateSystemType();

    std::string upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (!coordinate_system_type_to_enum.count(upper))
      mooseError("Unknown coordinate system type");

    return coordinate_system_type_to_enum[upper];
  }

  template<>
  PPSOutputType stringToEnum<PPSOutputType>(const std::string & s)
  {
    initPPSOutputType();

    std::string upper(s);
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (!pps_output_type_to_enum.count(upper))
      mooseError("Unknown PPS output type");

    return pps_output_type_to_enum[upper];
  }
}

Point toPoint(const std::vector<Real> & pos)
{
  mooseAssert(pos.size() == LIBMESH_DIM, "Wrong array size while converting into a point");
  return Point(pos[0], pos[1], pos[2]);
}
