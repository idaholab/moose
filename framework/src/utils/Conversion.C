//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Conversion.h"
#include "MooseError.h"
#include "ExecFlagEnum.h"
#include "MooseUtils.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/point.h"

// system includes
#include <iomanip>

namespace Moose
{
std::map<std::string, QuadratureType> quadrature_type_to_enum;
std::map<std::string, CoordinateSystemType> coordinate_system_type_to_enum;
std::map<std::string, SolveType> solve_type_to_enum;
std::map<std::string, EigenSolveType> eigen_solve_type_to_enum;
std::map<std::string, EigenProblemType> eigen_problem_type_to_enum;
std::map<std::string, WhichEigenPairs> which_eigen_pairs_to_enum;
std::map<std::string, LineSearchType> line_search_type_to_enum;
std::map<std::string, TimeIntegratorType> time_integrator_to_enum;
std::map<std::string, MffdType> mffd_type_to_enum;
std::map<std::string, RelationshipManagerType> rm_type_to_enum;

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
    eigen_solve_type_to_enum["NONLINEAR_POWER"] = EST_NONLINEAR_POWER;
    eigen_solve_type_to_enum["NEWTON"] = EST_NEWTON;
    eigen_solve_type_to_enum["PJFNK"] = EST_PJFNK;
    eigen_solve_type_to_enum["PJFNKMO"] = EST_PJFNKMO;
    eigen_solve_type_to_enum["JFNK"] = EST_JFNK;
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
    eigen_problem_type_to_enum["SLEPC_DEFAULT"] = EPT_SLEPC_DEFAULT;
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
    which_eigen_pairs_to_enum["SLEPC_DEFAULT"] = WEP_SLEPC_DEFAULT;
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

    line_search_type_to_enum["SHELL"] = LS_SHELL;
    line_search_type_to_enum["L2"] = LS_L2;
    line_search_type_to_enum["BT"] = LS_BT;
    line_search_type_to_enum["CP"] = LS_CP;
    line_search_type_to_enum["CONTACT"] = LS_CONTACT;
    line_search_type_to_enum["PROJECT"] = LS_PROJECT;
  }
}

void
initTimeIntegratorsType()
{
  if (time_integrator_to_enum.empty())
  {
    time_integrator_to_enum["IMPLICIT_EULER"] = TI_IMPLICIT_EULER;
    time_integrator_to_enum["EXPLICIT_EULER"] = TI_EXPLICIT_EULER;
    time_integrator_to_enum["CRANK_NICOLSON"] = TI_CRANK_NICOLSON;
    time_integrator_to_enum["BDF2"] = TI_BDF2;
    time_integrator_to_enum["EXPLICIT_MIDPOINT"] = TI_EXPLICIT_MIDPOINT;
    time_integrator_to_enum["LSTABLE_DIRK2"] = TI_LSTABLE_DIRK2;
    time_integrator_to_enum["EXPLICIT_TVDRK2"] = TI_EXPLICIT_TVD_RK_2;
  }
}

void
initMffdType()
{
  if (mffd_type_to_enum.empty())
  {
    mffd_type_to_enum["DS"] = MFFD_DS;
    mffd_type_to_enum["WP"] = MFFD_WP;
  }
}

void
initRMType()
{
  if (rm_type_to_enum.empty())
  {
    rm_type_to_enum["DEFAULT"] = RelationshipManagerType::DEFAULT;
    rm_type_to_enum["GEOMETRIC"] = RelationshipManagerType::GEOMETRIC;
    rm_type_to_enum["ALGEBRAIC"] = RelationshipManagerType::ALGEBRAIC;
    rm_type_to_enum["COUPLING"] = RelationshipManagerType::COUPLING;
  }
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
TimeIntegratorType
stringToEnum<TimeIntegratorType>(const std::string & s)
{
  initTimeIntegratorsType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!time_integrator_to_enum.count(upper))
    mooseError("Unknown time integrator: ", upper);

  return time_integrator_to_enum[upper];
}

template <>
MffdType
stringToEnum<MffdType>(const std::string & s)
{
  initMffdType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!mffd_type_to_enum.count(upper))
    mooseError("Unknown mffd type: ", upper);

  return mffd_type_to_enum[upper];
}

template <>
RelationshipManagerType
stringToEnum<RelationshipManagerType>(const std::string & s)
{
  initRMType();

  std::string upper(s);
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

  if (!rm_type_to_enum.count(upper))
    mooseError("Unknown RelationshipManager type: ", upper);

  return rm_type_to_enum[upper];
}

// Ignore warnings about switching on the |'d type
#include "libmesh/ignore_warnings.h"

// Definition in MooseTypes.h
std::string
stringify(const RelationshipManagerType & t)
{
  // Cannot make a switch statement because the boolean logic doesn't work well with the class type
  // enumeration and because Cody says so.
  if (t == RelationshipManagerType::DEFAULT)
    return "DEFAULT";
  if (t == RelationshipManagerType::GEOMETRIC)
    return "GEOMETRIC";
  if (t == RelationshipManagerType::ALGEBRAIC)
    return "ALGEBRAIC";
  if (t == (RelationshipManagerType::GEOMETRIC | RelationshipManagerType::ALGEBRAIC))
    return "GEOMETRIC and ALGEBRAIC";
  if (t == (RelationshipManagerType::ALGEBRAIC | RelationshipManagerType::COUPLING))
    return "ALGEBRAIC and COUPLING";
  if (t == (RelationshipManagerType::GEOMETRIC | RelationshipManagerType::ALGEBRAIC |
            RelationshipManagerType::COUPLING))
    return "GEOMETRIC and ALGEBRAIC and COUPLING";
  if (t == RelationshipManagerType::COUPLING)
    return "COUPLING";

  mooseError("Unknown RelationshipManagerType");
}

std::string
stringify(FEFamily f)
{
  switch (f)
  {
    case 0:
      return "LAGRANGE";
    case 1:
      return "HIERARCHIC";
    case 2:
      return "MONOMIAL";
    case 6:
      return "L2_HIERARCHIC";
    case 7:
      return "L2_LAGRANGE";
    case 3:
      return "BERNSTEIN";
    case 4:
      return "SZABAB";
    case 5:
      return "XYZ";
    case 11:
      return "INFINITE_MAP";
    case 12:
      return "JACOBI_20_00";
    case 13:
      return "JACOBI_30_00";
    case 14:
      return "LEGENDRE";
    case 21:
      return "CLOUGH";
    case 22:
      return "HERMITE";
    case 23:
      return "SUBDIVISION";
    case 31:
      return "SCALAR";
    case 41:
      return "LAGRANGE_VEC";
    case 42:
      return "NEDELEC_ONE";
    case 43:
      return "MONOMIAL_VEC";
    case 99:
      return "INVALID_FE";
    default:
      mooseError("Unrecognized FEFamily ", static_cast<int>(f));
  }
}

// Turn the warnings back on
#include "libmesh/restore_warnings.h"

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

std::string
stringify(const EigenSolveType & t)
{
  switch (t)
  {
    case EST_POWER:
      return "Power";
    case EST_ARNOLDI:
      return "ARNOLDI";
    case EST_KRYLOVSCHUR:
      return "KRYLOVSCHUR";
    case EST_JACOBI_DAVIDSON:
      return "Jacobi Davidson";
    case EST_NONLINEAR_POWER:
      return "Nonlinear Power";
    case EST_PJFNKMO:
      return "PJFNK with Matrix Only";
    case EST_NEWTON:
      return "Newton";
    case EST_JFNK:
      return "JFNK";
    case EST_PJFNK:
      return "Preconditioned JFNK";
  }
  return "";
}

std::string
stringify(const VarFieldType & t)
{
  switch (t)
  {
    case VAR_FIELD_STANDARD:
      return "STANDARD";
    case VAR_FIELD_VECTOR:
      return "VECTOR";
    case VAR_FIELD_ARRAY:
      return "ARRAY";
    case VAR_FIELD_SCALAR:
      return "SCALAR";
    case VAR_FIELD_ANY:
      return "ANY";
  }
  return "";
}

std::string
stringify(SolutionIterationType t)
{
  switch (t)
  {
    case SolutionIterationType::Time:
      return "time";
    case SolutionIterationType::Nonlinear:
      return "nonlinear";
    default:
      mooseError("Unhandled SolutionIterationType");
  }
}

std::string
stringify(ElementType t)
{
  switch (t)
  {
    case ElementType::Element:
      return "ELEMENT";
    case ElementType::Neighbor:
      return "NEIGHBOR";
    case ElementType::Lower:
      return "LOWER";
    default:
      mooseError("unrecognized type");
  }
}

std::string
stringify(const std::string & s)
{
  return s;
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

Point
toPoint(const std::vector<Real> & pos)
{
  mooseAssert(pos.size() == LIBMESH_DIM, "Wrong array size while converting into a point");
  return Point(pos[0], pos[1], pos[2]);
}
}
