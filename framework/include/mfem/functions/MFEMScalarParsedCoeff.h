/***********************************************\
!        Construct a non-linear parsed          !
!        grid function coefficient              !
!                                               !
\***********************************************/

// Define a coefficient that, given a grid function u,
// function func, returns func(u)

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMContainers.h"
#include "FunctionParserUtils.h"
#include <vector>
#include <functional>

class MFEMScalarParsedCoeff : public mfem::Coefficient
{
private:
  const Moose::MFEM::GridFunctions & _gFuncs;
  const std::vector<std::string> & _inputs;
  bool _use_xyzt;
  const FunctionParserUtils<false>::SymFunctionPtr & _func;

public:
  MFEMScalarParsedCoeff(const Moose::MFEM::GridFunctions & gFuncs,
                        const std::vector<std::string> & inputs,
                        bool use_xyzt,
                        const FunctionParserUtils<false>::SymFunctionPtr & func);

  double Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip);
};

#endif
