/***********************************************\
!        Construct a non-linear parsed          !
!        grid function coefficient              !
!                                               !
\***********************************************/

// Define a coefficient that, given a grid function u,
// function func, returns func(u)

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarParsedCoeff.h"

MFEMScalarParsedCoeff::MFEMScalarParsedCoeff(
    const Moose::MFEM::GridFunctions & gFuncs,
    const std::vector<std::string> & inputs,
    bool use_xyzt,
    const FunctionParserUtils<false>::SymFunctionPtr & func)
  : _gFuncs(gFuncs), _inputs(inputs), _use_xyzt(use_xyzt), _func(func)
{
}

double
MFEMScalarParsedCoeff::Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip)
{
  std::vector<libMesh::Real> inpVals(_inputs.size() + (_use_xyzt ? 4 : 0));

  for (unsigned i = 0; i < _inputs.size(); i++)
    inpVals[i] = _gFuncs.GetRef(_inputs[i]).GetValue(T, ip);

  if (_use_xyzt)
  {
    mfem::Vector transip(3);
    T.Transform(ip, transip);

    for (unsigned i = 0; i < 3; i++)
      inpVals[_inputs.size() + i] = transip(i);

    inpVals[_inputs.size() + 3] = GetTime();
  }

  return _func->Eval(inpVals.data());
}

#endif
