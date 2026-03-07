//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParameterRegistration.h"

#include "ParameterRegistry.h"
#include "MooseTypes.h"
#include "ReporterName.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "ExecFlagEnum.h"

namespace Moose::ParameterRegistration
{

template <>
void
setScalarValue(bool & value, const hit::Field & field)
{
  // Handles non-quoted values
  try
  {
    value = field.boolVal();
  }
  // Handles quoted values
  catch (hit::Error &)
  {
    const auto strval = field.param<std::string>();
    if (strval == "1")
      value = true;
    else if (strval == "0")
      value = false;
    else if (!hit::toBool(strval, &value))
      throw std::invalid_argument("invalid boolean syntax for parameter: " + field.path() + "='" +
                                  strval + "'");
  }
}

/*******************************************************************************/
/* This contains the initialization time registration of all of the base       */
/* parameter types that are registered for use in the Builder for MOOSE.       */
/*******************************************************************************/

namespace detail
{

static Moose::ParameterRegistry & registry = Moose::ParameterRegistry::get();

/*******************************************************************************/
/* Derivative string registration                                              */
/*                                                                             */
/* Registers for scalars, vectors, double vectors, and triple vectors          */
/*                                                                             */
/* See MooseDerivativeStringClass macro usage in MooseTypes.h                  */
/* Please keep the ordering consistent                                         */
/*******************************************************************************/

// Special cases that aren't included: CLIArgString
registerParameter(FileName);
registerParameter(FileNameNoExtension);
registerParameter(RelativeFileName);
registerParameter(DataFileName);
registerParameter(MeshFileName);
registerParameter(MatrixFileName);
registerParameter(OutFileBase);
registerParameter(NonlinearVariableName);
registerParameter(LinearVariableName);
registerParameter(SolverVariableName);
registerParameter(AuxVariableName);
registerParameter(VariableName);
registerParameter(BoundaryName);
registerParameter(SubdomainName);
registerParameter(PostprocessorName);
registerParameter(VectorPostprocessorName);
registerParameter(MeshDivisionName);
registerParameter(FunctionName);
registerParameter(DistributionName);
registerParameter(SamplerName);
registerParameter(UserObjectName);
registerParameter(InterpolationMethodName);
registerParameter(IndicatorName);
registerParameter(MarkerName);
registerParameter(MultiAppName);
registerParameter(OutputName);
registerParameter(MaterialPropertyName);
registerParameter(MooseFunctorName);
registerParameter(MaterialName);
registerParameter(TagName);
registerParameter(MeshGeneratorName);
registerParameter(ExtraElementIDName);
registerParameter(ReporterValueName);
registerParameter(ComponentName);
registerParameter(PhysicsName);
registerParameter(PositionsName);
registerParameter(TimesName);
registerParameter(ExecutorName);
registerParameter(ParsedFunctionExpression);
registerParameter(NonlinearSystemName);
registerParameter(ConvergenceName);
registerParameter(LinearSystemName);
registerParameter(SolverSystemName);
#ifdef MOOSE_MFEM_ENABLED
registerParameter(MFEMScalarCoefficientName);
registerParameter(MFEMVectorCoefficientName);
registerParameter(MFEMMatrixCoefficientName);
#endif

/*******************************************************************************/
/* Native type registration                                                    */
/*                                                                             */
/* Registers for scalars, vectors, double vectors, and triple vectors          */
/*******************************************************************************/

registerParameter(double);
registerParameter(std::string);
registerParameter(short int);
registerParameter(int);
registerParameter(long int);
registerParameter(unsigned short);
registerParameter(unsigned int);
registerParameter(unsigned long);
registerParameter(unsigned long long);

/*******************************************************************************/
/* Bool registration                                                           */
/*******************************************************************************/

registerScalarParameter(bool);
registerVectorParameter(bool);

/*******************************************************************************/
/* Helpers used across types                                                   */
/*******************************************************************************/

/// Helper for converting a string to a ReporterName, which requires splitting
/// the string at the '/' delimiter
const auto convert_reporter_name = [](const std::string & val,
                                      const hit::Field & field) -> ReporterName
{
  const auto names = MooseUtils::rsplit(val, "/", 2);
  if (names.size() != 2)
    throw std::invalid_argument("invalid syntax in ReporterName parameter " + field.fullpath() +
                                ": supplied name '" + val + "' must contain the '/' delimiter");
  return {names[0], names[1]};
};

/*******************************************************************************/
/* Component-typed scalar registration                                         */
/*******************************************************************************/

/// Helper for setting a scalar component value (one with 3 real values)
const auto set_scalar_component_value = [](auto & value, const hit::Field & field)
{
  const auto vec = field.param<std::vector<double>>();
  if (vec.size() != LIBMESH_DIM)
    throw std::invalid_argument("wrong number of values in " + MooseUtils::prettyCppType(&value) +
                                " parameter '" + field.fullpath() + "': was given " +
                                std::to_string(vec.size()) + " component(s) but should have " +
                                std::to_string(LIBMESH_DIM));

  for (const auto d : make_range(LIBMESH_DIM))
    value(d) = Real(vec[d]);
};

// Point
static auto point = registry.add<Point>([](Point & value, const hit::Field & field)
                                        { set_scalar_component_value(value, field); });
// RealVectorValue
static auto realvectorvalue =
    registry.add<RealVectorValue>([](RealVectorValue & value, const hit::Field & field)
                                  { set_scalar_component_value(value, field); });

/*******************************************************************************/
/* Custom scalar registration                                                  */
/*******************************************************************************/

// We only support CLIArgString and std::vector<CLIArgString>, so this isn't
// added in the main MooseDerivativeString case
registerScalarParameter(CLIArgString);
// RealEigenVector
static auto realeigenvector = registry.add<RealEigenVector>(
    [](RealEigenVector & value, const hit::Field & field)
    {
      const auto vec = field.param<std::vector<double>>();
      value.resize(vec.size());
      for (const auto i : index_range(vec))
        value(i) = Real(vec[i]);
    });
// RealEigenVector
static auto realeigenmatrix = registry.add<RealEigenMatrix>(
    [](RealEigenMatrix & value, const hit::Field & field)
    {
      value.resize(0, 0);
      std::vector<std::string> tokens;
      MooseUtils::tokenize(field.param<std::string>(), tokens, 1, ";");

      for (const auto i : index_range(tokens))
      {
        const auto token = MooseUtils::trim(tokens[i]);
        std::vector<Real> values;
        if (!MooseUtils::tokenizeAndConvert<Real>(token, values))
          throw std::invalid_argument("invalid syntax for parameter: " + field.fullpath() + "[" +
                                      std::to_string(i) + "]='" + token + "'");
        if (i == 0)
          value.resize(tokens.size(), values.size());
        else if (libMesh::cast_int<std::size_t>(value.cols()) != values.size())
          throw std::invalid_argument("matrix is not square for parameter " + field.fullpath());
        for (const auto j : index_range(values))
          value(i, j) = values[j];
      }
    });
// MooseEnum
static auto mooseenum = registry.add<MooseEnum>([](MooseEnum & value, const hit::Field & field)
                                                { value = field.param<std::string>(); });
// MultiMooseEnum
static auto multimooseenum = registry.add<MultiMooseEnum>(
    [](MultiMooseEnum & value, const hit::Field & field)
    { value = MooseUtils::stringJoin(field.param<std::vector<std::string>>(), " "); });
// ExecFlagEnum
static auto execflagenum = registry.add<ExecFlagEnum>(
    [](ExecFlagEnum & value, const hit::Field & field)
    { value = MooseUtils::stringJoin(field.param<std::vector<std::string>>(), " "); });
// RealTensorValue
static auto realtensorvalue = registry.add<RealTensorValue>(
    [](RealTensorValue & value, const hit::Field & field)
    {
      const auto vec = field.param<std::vector<double>>();
      if (vec.size() != LIBMESH_DIM * LIBMESH_DIM)
        throw std::invalid_argument("invalid RealTensorValue parameter '" + field.fullpath() +
                                    "': size is " + std::to_string(vec.size()) + " but should be " +
                                    std::to_string(LIBMESH_DIM * LIBMESH_DIM));

      for (const auto i : make_range(LIBMESH_DIM))
        for (const auto j : make_range(LIBMESH_DIM))
          value(i, j) = Real(vec[i * LIBMESH_DIM + j]);
    });
// ReporterName
static auto reportername = registry.add<ReporterName>(
    [](ReporterName & value, const hit::Field & field)
    { value = convert_reporter_name(field.param<std::string>(), field); });

/*******************************************************************************/
/* Component-typed vector registration                                         */
/*******************************************************************************/

/// Helper for setting a vector component value (one with 3 real values)
const auto set_vector_component_value = [](auto & value, const hit::Field & field)
{
  value.clear();
  const auto vec = field.param<std::vector<double>>();

  if (vec.size() % LIBMESH_DIM)
    throw std::invalid_argument("wrong number of values in vector parameter '" + field.fullpath() +
                                "': size " + std::to_string(vec.size()) + " is not a multiple of " +
                                std::to_string(LIBMESH_DIM));

  const std::size_t size = vec.size() / LIBMESH_DIM;
  value.resize(size);
  for (const auto i : make_range(size))
    for (const auto d : make_range(LIBMESH_DIM))
      value[i](d) = vec[i * LIBMESH_DIM + d];
};

// std::vector<Point>
static auto vector_point =
    registry.add<std::vector<Point>>([](std::vector<Point> & value, const hit::Field & field)
                                     { set_vector_component_value(value, field); });
// std::vector<RealVectorValue>
static auto vector_realvectorvalue = registry.add<std::vector<RealVectorValue>>(
    [](std::vector<RealVectorValue> & value, const hit::Field & field)
    { set_vector_component_value(value, field); });

/*******************************************************************************/
/* Custom vector registration                                                  */
/*******************************************************************************/

// std::vector<MooseEnum>
static auto vector_mooseenum = registry.add<std::vector<MooseEnum>>(
    [](std::vector<MooseEnum> & value, const hit::Field & field)
    {
      mooseAssert(!value.empty(), "Missing a value to initialize on");

      // With MOOSE enums we need a default object so it should have been
      // passed in the param pointer. We are only going to use the first
      // item in the vector (value[0]) and ignore the rest.
      const auto vec = field.param<std::vector<std::string>>();
      value.resize(vec.size(), value[0]);
      for (const auto i : index_range(vec))
        value[i] = vec[i];
    });
// std::vector<MooseEnum>
static auto vector_multimooseenum = registry.add<std::vector<MultiMooseEnum>>(
    [](std::vector<MultiMooseEnum> & value, const hit::Field & field)
    {
      mooseAssert(!value.empty(), "Missing a value to initialize on");

      const auto tokens = MooseUtils::split(field.param<std::string>(), ";");
      for (const auto i : index_range(tokens))
      {
        const auto & entry = tokens[i];
        if (MooseUtils::trim(entry) == "")
          throw std::invalid_argument("entry " + std::to_string(i) + " in '" + field.fullpath() +
                                      "' is empty");
      }
      value.resize(tokens.size(), value[0]);
      for (const auto i : index_range(tokens))
      {
        std::vector<std::string> strvals;
        MooseUtils::tokenize<std::string>(tokens[i], strvals, 1, " ");
        value[i] = strvals;
      }
    });
// std::vector<ReporterName>
static auto vector_reportername = registry.add<std::vector<ReporterName>>(
    [](std::vector<ReporterName> & value, const hit::Field & field)
    {
      value.clear();
      const auto names = field.param<std::vector<std::string>>();
      value.resize(names.size());
      for (const auto i : index_range(names))
        value[i] = convert_reporter_name(names[i], field);
    });
// std::vector<CLIArgString>
static auto vector_cliargstring = registry.add<std::vector<CLIArgString>>(
    [](std::vector<CLIArgString> & value, const hit::Field & field)
    {
      value.clear();
      const auto strvals = field.param<std::vector<std::string>>();
      if (strvals.empty())
        return;

      // slightly oversized if vectors have been split
      value.resize(strvals.size());

      // Re-assemble vector parameters
      unsigned int i_param = 0;
      bool vector_param_detected = false;
      for (const auto i : index_range(strvals))
      {
        // Look for a quote, both types
        const auto double_split =
            MooseUtils::rsplit(strvals[i], "\"", std::numeric_limits<std::size_t>::max());
        const auto single_split =
            MooseUtils::rsplit(strvals[i], "\'", std::numeric_limits<std::size_t>::max());
        if (double_split.size() + single_split.size() >= 3)
          // Either entering or exiting a vector parameter (>3 is entering another vector)
          // Even and >2 number of quotes means both finished and started another vector parameter
          if ((double_split.size() + single_split.size()) % 2 == 1)
            vector_param_detected = !vector_param_detected;

        // We're building a vector parameters, just append the text, rebuild the spaces
        if (vector_param_detected)
          value[i_param] += strvals[i] + ' ';
        else
        {
          value[i_param] += strvals[i];
          i_param++;
        }
      }
      // Use actual size after re-forming vector parameters
      value.resize(i_param);
    });

/*******************************************************************************/
/* Component-typed double vector registration                                  */
/*******************************************************************************/

/// Helper for setting a double vector component value (one with 3 real values)
const auto set_double_vector_component_value = [](auto & value, const hit::Field & field)
{
  const auto strval = field.param<std::string>();

  // Split vector at delim ";" (substrings are not of type T yet)
  std::vector<std::string> tokens;
  MooseUtils::tokenize(strval, tokens, 1, ";");
  value.resize(tokens.size());

  // Split each token at spaces
  std::vector<std::vector<double>> vecvec(tokens.size());
  for (const auto i : index_range(tokens))
    if (!MooseUtils::tokenizeAndConvert<double>(tokens[i], vecvec[i]))
      throw std::invalid_argument("invalid format for parameter '" + field.fullpath() +
                                  "' at index " + std::to_string(i));

  for (const auto i : index_range(vecvec))
  {
    const auto & vec = vecvec[i];
    if (vec.size() % LIBMESH_DIM)
      throw std::invalid_argument(
          "wrong number of values in double-indexed vector component parameter '" +
          field.fullpath() + "' at index " + std::to_string(i) + ": subcomponent size " +
          std::to_string(vec.size()) + " is not a multiple of " + std::to_string(LIBMESH_DIM));
  }

  // convert vector<vector<double>> to vector<vector<T>>
  value.resize(vecvec.size());
  for (const auto i : index_range(vecvec))
  {
    const auto & vec_entry = vecvec[i];
    auto & value_entry = value[i];
    const std::size_t size = vec_entry.size() / LIBMESH_DIM;
    value_entry.resize(size);
    for (const auto j : make_range(size))
      for (const auto d : make_range(LIBMESH_DIM))
        value_entry[j](d) = vec_entry[j * LIBMESH_DIM + d];
  }
};

// std::vector<std::vector<Point>>
static auto doublevector_point = registry.add<std::vector<std::vector<Point>>>(
    [](std::vector<std::vector<Point>> & value, const hit::Field & field)
    { set_double_vector_component_value(value, field); });
// std::vector<std::vector<RealVectorValue>>
static auto doublevector_realvectorvalue = registry.add<std::vector<std::vector<RealVectorValue>>>(
    [](std::vector<std::vector<RealVectorValue>> & value, const hit::Field & field)
    { set_double_vector_component_value(value, field); });

/*******************************************************************************/
/* Map registration                                                            */
/*******************************************************************************/

registerMapParameter(std::string, unsigned int);
registerMapParameter(std::string, Real);
registerMapParameter(std::string, std::string);
registerMapParameter(unsigned int, unsigned int);
registerMapParameter(unsigned long, unsigned int);
registerMapParameter(unsigned long long, unsigned int);

} // end of namespace detail
} // end of namespace Moose::ParameterRegistration
