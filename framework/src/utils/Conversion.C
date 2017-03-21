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

// MOOSE includes
#include "Conversion.h"
#include "MooseError.h"
#include "MultiMooseEnum.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

// system includes
#include <iomanip>

namespace Moose
{
std::map<std::string, ExecFlagType> execstore_type_to_enum;
std::map<std::string, QuadratureType> quadrature_type_to_enum;
std::map<std::string, CoordinateSystemType> coordinate_system_type_to_enum;
std::map<std::string, SolveType> solve_type_to_enum;
std::map<std::string, EigenSolveType> eigen_solve_type_to_enum;
std::map<std::string, EigenProblemType> eigen_problem_type_to_enum;
std::map<std::string, WhichEigenPairs> which_eigen_pairs_to_enum;
std::map<std::string, LineSearchType> line_search_type_to_enum;

void
initExecStoreType()
{
  if (execstore_type_to_enum.empty())
  {
    execstore_type_to_enum["INITIAL"] = EXEC_INITIAL;
    execstore_type_to_enum["LINEAR"] = EXEC_LINEAR;
    execstore_type_to_enum["NONLINEAR"] = EXEC_NONLINEAR;
    execstore_type_to_enum["TIMESTEP_END"] = EXEC_TIMESTEP_END;
    execstore_type_to_enum["TIMESTEP_BEGIN"] = EXEC_TIMESTEP_BEGIN;
    execstore_type_to_enum["CUSTOM"] = EXEC_CUSTOM;
  }
}

void
initQuadratureType()
{
  if (quadrature_type_to_enum.empty())
  {
    quadrature_type_to_enum["CLOUGH"] = QCLOUGH;
    quadrature_type_to_enum["CONICAL"] = QCONICAL;
    quadrature_type_to_enum["GAUSS"] = QGAUSS;
    quadrature_type_to_enum["GRID"] = QGRID;
    quadrature_type_to_enum["MONOMIAL"] = QMONOMIAL;
    quadrature_type_to_enum["SIMPSON"] = QSIMPSON;
    quadrature_type_to_enum["TRAP"] = QTRAP;
    quadrature_type_to_enum["GAUSS_LOBATTO"] = QGAUSS_LOBATTO;
  }
}

void
initCoordinateSystemType()
{
  if (coordinate_system_type_to_enum.empty())
  {
    coordinate_system_type_to_enum["XYZ"] = COORD_XYZ;
    coordinate_system_type_to_enum["RZ"] = COORD_RZ;
    coordinate_system_type_to_enum["RSPHERICAL"] = COORD_RSPHERICAL;
  }
}

void
initSolveType()
{
  if (solve_type_to_enum.empty())
  {
    solve_type_to_enum["PJFNK"] = ST_PJFNK;
    solve_type_to_enum["JFNK"] = ST_JFNK;
    solve_type_to_enum["NEWTON"] = ST_NEWTON;
    solve_type_to_enum["FD"] = ST_FD;
    solve_type_to_enum["LINEAR"] = ST_LINEAR;
  }
}

void
initEigenSolveType()
{
  if (eigen_solve_type_to_enum.empty())
  {
    eigen_solve_type_to_enum["POWER"] = EST_POWER;
    eigen_solve_type_to_enum["ARNOLDI"] = EST_ARNOLDI;
    eigen_solve_type_to_enum["KRYLOVSCHUR"] = EST_KRYLOVSCHUR;
    eigen_solve_type_to_enum["JACOBI_DAVIDSON"] = EST_JACOBI_DAVIDSON;
  }
}

void
initEigenProlemType()
{
  if (eigen_problem_type_to_enum.empty())
  {
    eigen_problem_type_to_enum["HERMITIAN"] = EPT_HERMITIAN;
    eigen_problem_type_to_enum["NON_HERMITIAN"] = EPT_NON_HERMITIAN;
    eigen_problem_type_to_enum["GEN_HERMITIAN"] = EPT_GEN_HERMITIAN;
    eigen_problem_type_to_enum["GEN_NON_HERMITIAN"] = EPT_GEN_NON_HERMITIAN;
    eigen_problem_type_to_enum["GEN_INDEFINITE"] = EPT_GEN_INDEFINITE;
    eigen_problem_type_to_enum["POS_GEN_NON_HERMITIAN"] = EPT_POS_GEN_NON_HERMITIAN;
  }
}

void
initWhichEigenPairs()
{
  if (which_eigen_pairs_to_enum.empty())
  {
    which_eigen_pairs_to_enum["LARGEST_MAGNITUDE"] = WEP_LARGEST_MAGNITUDE;
    which_eigen_pairs_to_enum["SMALLEST_MAGNITUDE"] = WEP_SMALLEST_MAGNITUDE;
    which_eigen_pairs_to_enum["LARGEST_REAL"] = WEP_LARGEST_REAL;
    which_eigen_pairs_to_enum["SMALLEST_REAL"] = WEP_SMALLEST_REAL;
    which_eigen_pairs_to_enum["LARGEST_IMAGINARY"] = WEP_LARGEST_IMAGINARY;
    which_eigen_pairs_to_enum["SMALLEST_IMAGINARY"] = WEP_SMALLEST_IMAGINARY;
    which_eigen_pairs_to_enum["TARGET_MAGNITUDE"] = WEP_TARGET_MAGNITUDE;
    which_eigen_pairs_to_enum["TARGET_REAL"] = WEP_TARGET_REAL;
    which_eigen_pairs_to_enum["TARGET_IMAGINARY"] = WEP_TARGET_IMAGINARY;
    which_eigen_pairs_to_enum["ALL_EIGENVALUES"] = WEP_ALL_EIGENVALUES;
  }
}

void
initLineSearchType()
{
  if (line_search_type_to_enum.empty())
  {
    line_search_type_to_enum["DEFAULT"] = LS_DEFAULT;
    line_search_type_to_enum["NONE"] = LS_NONE;
    line_search_type_to_enum["BASIC"] = LS_BASIC;
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
    line_search_type_to_enum["CUBIC"] = LS_CUBIC;
    line_search_type_to_enum["QUADRATIC"] = LS_QUADRATIC;
    line_search_type_to_enum["BASICNONORMS"] = LS_BASICNONORMS;
#else
    line_search_type_to_enum["SHELL"] = LS_SHELL;
    line_search_type_to_enum["L2"] = LS_L2;
    line_search_type_to_enum["BT"] = LS_BT;
    line_search_type_to_enum["CP"] = LS_CP;
#endif
#endif
  }
}

template <>
ExecFlagType
stringToEnum(const std::string & s)
{
  initExecStoreType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!execstore_type_to_enum.count(upper))
    mooseError("Unknown execution flag: ", upper);

  return execstore_type_to_enum[upper];
}

template <>
QuadratureType
stringToEnum<QuadratureType>(const std::string & s)
{
  initQuadratureType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!quadrature_type_to_enum.count(upper))
    mooseError("Unknown quadrature type: ", upper);

  return quadrature_type_to_enum[upper];
}

template <>
Order
stringToEnum<Order>(const std::string & s)
{
  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (upper.compare("AUTO") == 0)
    return INVALID_ORDER;
  else
    return Utility::string_to_enum<Order>(upper);
}

template <>
CoordinateSystemType
stringToEnum<CoordinateSystemType>(const std::string & s)
{
  initCoordinateSystemType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!coordinate_system_type_to_enum.count(upper))
    mooseError("Unknown coordinate system type: ", upper);

  return coordinate_system_type_to_enum[upper];
}

template <>
SolveType
stringToEnum<SolveType>(const std::string & s)
{
  initSolveType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!solve_type_to_enum.count(upper))
    mooseError("Unknown solve type: ", upper);

  return solve_type_to_enum[upper];
}

template <>
EigenSolveType
stringToEnum<EigenSolveType>(const std::string & s)
{
  initEigenSolveType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!eigen_solve_type_to_enum.count(upper))
    mooseError("Unknown eigen solve type: ", upper);

  return eigen_solve_type_to_enum[upper];
}

template <>
EigenProblemType
stringToEnum<EigenProblemType>(const std::string & s)
{
  initEigenProlemType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!eigen_problem_type_to_enum.count(upper))
    mooseError("Unknown eigen problem type: ", upper);

  return eigen_problem_type_to_enum[upper];
}

template <>
WhichEigenPairs
stringToEnum<WhichEigenPairs>(const std::string & s)
{
  initWhichEigenPairs();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!which_eigen_pairs_to_enum.count(upper))
    mooseError("Unknown type of WhichEigenPairs: ", upper);

  return which_eigen_pairs_to_enum[upper];
}

template <>
LineSearchType
stringToEnum<LineSearchType>(const std::string & s)
{
  initLineSearchType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!line_search_type_to_enum.count(upper))
    mooseError("Unknown line search type: ", upper);

  return line_search_type_to_enum[upper];
}

template <>
std::vector<ExecFlagType>
vectorStringsToEnum<ExecFlagType>(const MultiMooseEnum & v)
{
  std::vector<ExecFlagType> exec_flags(v.size());
  for (unsigned int i = 0; i < v.size(); ++i)
    exec_flags[i] = stringToEnum<ExecFlagType>(v[i]);

  return exec_flags;
}

template <>
std::string
stringify(const SolveType & t)
{
  switch (t)
  {
    case ST_NEWTON:
      return "NEWTON";
    case ST_JFNK:
      return "JFNK";
    case ST_PJFNK:
      return "Preconditioned JFNK";
    case ST_FD:
      return "FD";
    case ST_LINEAR:
      return "Linear";
  }
  return "";
}

template <>
std::string
stringify(const ExecFlagType & t)
{
  switch (t)
  {
    case EXEC_INITIAL:
      return "INITIAL";
    case EXEC_LINEAR:
      return "LINEAR";
    case EXEC_NONLINEAR:
      return "NONLINEAR";
    case EXEC_TIMESTEP_END:
      return "TIMESTEP_END";
    case EXEC_TIMESTEP_BEGIN:
      return "TIMESTEP_BEGIN";
    case EXEC_CUSTOM:
      return "CUSTOM";
    case EXEC_FINAL:
      return "FINAL";
    case EXEC_FORCED:
      return "FORCED";
    case EXEC_FAILED:
      return "FAILED";
    case EXEC_SUBDOMAIN:
      return "SUBDOMAIN";
    case EXEC_NONE:
      return "NONE";
  }
  return "";
}

std::string
stringifyExact(Real t)
{
  // this or std::numeric_limits<T>::max_digits10
  const unsigned int max_digits10 =
      std::floor(std::numeric_limits<Real>::digits * std::log10(2) + 2);

  std::ostringstream os;
  os << std::setprecision(max_digits10) << t;
  return os.str();
}
}

Point
toPoint(const std::vector<Real> & pos)
{
  mooseAssert(pos.size() == LIBMESH_DIM, "Wrong array size while converting into a point");
  return Point(pos[0], pos[1], pos[2]);
}
