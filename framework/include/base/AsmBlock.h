/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ASMBLOCK_H
#define ASMBLOCK_H

#include <vector>
#include "ParallelUniqueId.h"
// libMesh
#include "dof_map.h"
#include "dense_matrix.h"
#include "dense_vector.h"
#include "coupling_matrix.h"

class SystemBase;

class AsmBlock
{
public:
  AsmBlock(SystemBase & sys, CouplingMatrix * & cm, THREAD_ID tid);
  virtual ~AsmBlock();

  void init();

  void prepare(const Elem * elem);

  void addResidual(NumericVector<Number> & residual);
  void addJacobian(SparseMatrix<Number> & jacobian);

  DenseVector<Number> & residualBlock(unsigned int var_num) { return _sub_Re[var_num]; }
  DenseMatrix<Number> & jacobianBlock(unsigned int ivar, unsigned int jvar) { return _sub_Ke[ivar][jvar]; }

protected:
  SystemBase & _sys;
  CouplingMatrix * & _cm;
  const DofMap & _dof_map;                                      ///< DOF map
  THREAD_ID _tid;

  std::vector<std::vector<unsigned int> > _sub_dof_indices;     ///< DOF indices for each variable
  std::vector<DenseVector<Number> > _sub_Re;                    ///< residual contributions for each variable
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Ke;      ///< jacobian contributions

};

#endif /* ASMBLOCK_H */
