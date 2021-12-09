#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "MooseError.h"
#include <iomanip>

extern "C" void
umat_(double * stress,
      double * statev,
      double * ddsdde,
      double * sse,
      double * spd,
      double * scd,
      double * rpl,
      double * ddsddt,
      double * drplde,
      double * drpldt,
      double * stran,
      double * dstran,
      double * time,
      double * dtime,
      double * temp,
      double * dtemp,
      double * predef,
      double * dpred,
      char * cmname,
      int * ndi,
      int * nshr,
      int * ntens,
      int * nstatv,
      double * props,
      int * nprops,
      double * coords,
      double * drot,
      double * pnewdt,
      double * celent,
      double * dfgrd0,
      double * dfgrd1,
      int * noel,
      int * npt,
      int * layer,
      int * kspt,
      int * kstep,
      int * kinc,
      short cmname_len)
{
  const double E = props[0];
  const double nu = props[1];
  const double Ebulk = E / (1.0 - 2.0 * nu);
  const double Eg2 = E / (1.0 + nu);
  const double Eg = Eg2 / 2.0;
  const double Elam = (Ebulk - Eg2) / 3.0;

  // elastic stiffness
  for (int k1 = 0; k1 < *ndi; ++k1)
  {
    for (int k2 = 0; k2 < *ndi; ++k2)
      ddsdde[k1 * *ntens + k2] = Elam;
    ddsdde[k1 * *ntens + k1] += Eg2;
  }
  for (int k1 = *ndi; k1 < *ntens; ++k1)
    ddsdde[k1 * *ntens + k1] = Eg;

  // calculate stress
  for (int k1 = 0; k1 < *ntens; ++k1)
    for (int k2 = 0; k2 < *ntens; ++k2)
      stress[k1] += ddsdde[k1 * *ntens + k2] * dstran[k2];

  if (*npt == 8)
  {
    Moose::out << std::fixed << std::setprecision(7);
    for (int k1 = 0; k1 < *ntens; ++k1)
      Moose::out << "stran " << k1 << " = " << stran[k1] << "\n";

    for (int k1 = 0; k1 < *ntens; ++k1)
      Moose::out << "dstran " << k1 << " = " << dstran[k1] << "\n";

    for (int k1 = 0; k1 < *ntens; ++k1)
      Moose::out << "stress " << k1 << " = " << stress[k1] << "\n";

    for (int k1 = 0; k1 < *ndi; ++k1)
      Moose::out << "coords " << k1 << " = " << coords[k1] << "\n";

    for (int k1 = 0; k1 < *ndi * *ndi; ++k1)
      Moose::out << "dfgrd0 " << k1 << " = " << dfgrd0[k1] << "\n";

    for (int k1 = 0; k1 < *ndi * *ndi; ++k1)
      Moose::out << "dfgrd1 " << k1 << " = " << dfgrd1[k1] << "\n";

    for (int k1 = 0; k1 < *ndi * *ndi; ++k1)
      Moose::out << "drot " << k1 << " = " << drot[k1] << "\n";

    Moose::out << "time 0 = " << time[0] << "\n";
    Moose::out << "time 1 = " << time[1] << "\n";
    Moose::out << "celent = " << *celent << "\n";
    Moose::out << std::defaultfloat;
    Moose::out << "ndi = " << *ndi << "\n";
    Moose::out << "nshr = " << *nshr << "\n";
    Moose::out << "ntens = " << *ntens << "\n";
    Moose::out << "noel = " << *noel << "\n";
    Moose::out << "npt = " << *npt << "\n";
    Moose::out << "layer = " << *layer << "\n";
    Moose::out << "kspt = " << *kspt << "\n";
    Moose::out << "kstep = " << *kstep << "\n";
    Moose::out << "kinc = " << *kinc << "\n";
    Moose::out << "cmname = " << cmname << "\n";
  }
}

#pragma GCC diagnostic pop
