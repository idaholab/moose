/***************************************************************************************
**  UMAT, FOR ABAQUS/STANDARD INCORPORATING ISOTROPIC ELASTICITY                      **
***************************************************************************************/

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

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
  auto emod = props[0];
  auto enu = props[1];
  auto ebulk3 = emod / (1.0 - 2.0 * enu);
  auto eg2 = emod / (1.0 + enu);
  auto eg = eg2 / 2.0;
  auto elam = (ebulk3 - eg2) / 3.0;

  // elastic stiffness
  for (int k1 = 0; k1 < *ndi; ++k1)
  {
    for (int k2 = 0; k2 < *ndi; ++k2)
      ddsdde[k1 * *ntens + k2] = elam;
    ddsdde[k1 * *ntens + k1] += eg2;
  }
  for (int k1 = *ndi; k1 < *ntens; ++k1)
    ddsdde[k1 * *ntens + k1] = eg;

  // calculate stress
  for (int k1 = 0; k1 < *ntens; ++k1)
    for (int k2 = 0; k2 < *ntens; ++k2)
      stress[k1] += ddsdde[k1 * *ntens + k2] * dstran[k2];
}

#pragma GCC diagnostic pop
