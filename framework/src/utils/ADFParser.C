//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFParser.h"

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
  auto result = JITCompileHelper("ADReal", ADFPARSER_INCLUDES, "#include \"ADReal.h\"\n");

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
