//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFParser.h"
#include "MooseUtils.h"
#include "ExecutablePath.h"

ADFParser::ADFParser() : FunctionParserAD(), _epsilon(1e-12) {}

ADFParser::ADFParser(const ADFParser & cpy) : FunctionParserAD(cpy), _epsilon(1e-12) {}

#ifndef ADFPARSER_INCLUDES
#error ... \
       The ADFPARSER_INCLUDES macro is not defined. A possible reason is that you       \
       are compiling MOOSE from a custom application. Please check your application     \
       Makefile and make sure that you are appending options to ADDITIONAL_CPPFLAGS     \
       using the += operator, rather than overwriting the variable with the := operator.
#endif

bool
ADFParser::JITCompile()
{
#if LIBMESH_HAVE_FPARSER_JIT
  std::string includes;
  bool result;

  const auto include_path_env = std::getenv("MOOSE_ADFPARSER_JIT_INCLUDE");
  if (include_path_env)
    result = JITCompileHelper("ADReal", "", "#include \"" + std::string(include_path_env) + "\"\n");
  else
  {
    // check if we can find an installed version of the monolithic include
    const auto include_path =
        MooseUtils::pathjoin(Moose::getExecutablePath(), "../include/moose/ADRealMonolithic.h");
    if (MooseUtils::checkFileReadable(include_path, false, false, false))
      result = JITCompileHelper("ADReal", "", "#include \"" + include_path + "\"\n");
    else
      // otherwise use the compiled in location from the source tree
      result = JITCompileHelper("ADReal", ADFPARSER_INCLUDES, "#include \"ADReal.h\"\n");
  }

  if (!result)
#endif
    mooseError("ADFParser::JITCompile() failed. Evaluation not possible.");

  return true;
}

ADReal
ADFParser::Eval(const ADReal * vars)
{
  mooseAssert(compiledFunction, "ADFParser objects must be JIT compiled before evaluation!");
  ADReal ret;
  (*reinterpret_cast<CompiledFunctionPtr<ADReal>>(compiledFunction))(&ret, vars, pImmed, _epsilon);
  return ret;
}
