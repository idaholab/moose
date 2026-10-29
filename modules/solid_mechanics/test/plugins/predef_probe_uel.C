//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/***************************************************************************************
 **  UEL, FOR ABAQUS/STANDARD: A UNIT DIAGONAL STIFFNESS THAT REPORTS ITS PREDEF INPUT **
 ***************************************************************************************/

extern "C" void
uel_(double RHS[],
     double AMATRX[],
     double /*SVARS*/[],
     double ENERGY[],
     int * NDOFEL,
     int * /*NRHS*/,
     int * /*NSVARS*/,
     double /*PROPS*/[],
     int * /*NPROPS*/,
     double /*COORDS*/[],
     int * /*MCRD*/,
     int * NNODE,
     double U_[],
     double /*DU_*/[],
     double /*V_*/[],
     double /*A_*/[],
     int * /*JTYPE*/,
     double /*TIME*/[],
     double * /*DTIME*/,
     int * /*KSTEP*/,
     int * /*KINC*/,
     int * /*JELEM*/,
     double /*PRAMS*/[],
     int * /*NDLOAD*/,
     int /*JDLTYP*/[],
     double /*ADLMAG*/[],
     double PREDEF[],
     int * NPREDF,
     int /*LFLAGS*/[],
     int * /*MLVARX*/,
     double /*DDLMAG*/[],
     int * /*MDLOAD*/,
     double * /*PNEWDT*/,
     int /*JPROPS*/[],
     int * /*NJPROP*/,
     double * /*PERIOD*/)
{
  const int n = *NDOFEL;

  // u = 0 at every node: a unit diagonal stiffness with a zero right-hand side
  for (int i = 0; i < n; ++i)
  {
    RHS[i] = -U_[i];
    for (int j = 0; j < n; ++j)
      AMATRX[i * n + j] = (i == j) ? 1.0 : 0.0;
  }

  // PREDEF(K1, K2, K3) in Fortran order: K1 = 1 is the value and K1 = 2 the increment of field K2
  // at node K3. Report the nodal averages of field 1 in ENERGY(1) and ENERGY(2).
  double value = 0.0;
  double increment = 0.0;
  for (int k = 0; k < *NNODE; ++k)
  {
    value += PREDEF[2 * *NPREDF * k];
    increment += PREDEF[2 * *NPREDF * k + 1];
  }
  ENERGY[0] = value / *NNODE;
  ENERGY[1] = increment / *NNODE;
}
