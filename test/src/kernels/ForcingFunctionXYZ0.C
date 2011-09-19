#include "ForcingFunctionXYZ0.h"

template<>
InputParameters validParams<ForcingFunctionXYZ0>()
{
  InputParameters params = validParams<Kernel>();

  params.set<Real>("A0")=0.;
  params.set<Real>("B0")=0.;
  params.set<Real>("C0")=0.;

  params.set<Real>("Au")=1.;
  params.set<Real>("Bu")=1.;
  params.set<Real>("Cu")=1.;

  params.set<Real>("Av")=1.;
  params.set<Real>("Bv")=1.;
  params.set<Real>("Cv")=1.;

  params.set<Real>("Ak")=1.;
  params.set<Real>("Bk")=1.;
  params.set<Real>("Ck")=1.;

  params.set<Real>("omega0")=1.;

  return params;
}

ForcingFunctionXYZ0::ForcingFunctionXYZ0(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters)
  {
     _A0 = getParam<Real>("A0");
     _B0 = getParam<Real>("B0");
     _C0 = getParam<Real>("C0");

     _Au = getParam<Real>("Au");
     _Bu = getParam<Real>("Bu");
     _Cu = getParam<Real>("Cu");

     _Av = getParam<Real>("Av");
     _Bv = getParam<Real>("Bv");
     _Cv = getParam<Real>("Cv");

     _Ak = getParam<Real>("Ak");
     _Bk = getParam<Real>("Bk");
     _Ck = getParam<Real>("Ck");

     _omega0 = getParam<Real>("omega0");
  }

Real
ForcingFunctionXYZ0::computeQpResidual()
  {
     return _test[_i][_qp]*ManSol4ADR1src(_q_point[_qp],
                                         _A0,_B0,_C0,_Au,_Bu,_Cu,_Av,_Bv,_Cv,_Ak,_Bk,_Ck,
                                         _omega0,_t,_is_transient);
  }
