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
#include "MooseVariable.h"
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
  void prepareBlock(const Elem * elem, unsigned int ivar, unsigned jvar);

  void copyShapes(unsigned int var);
  void copyFaceShapes(unsigned int var);

  void addResidual(NumericVector<Number> & residual);
  void addJacobian(SparseMatrix<Number> & jacobian);
  void addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices);

  DenseVector<Number> & residualBlock(unsigned int var_num) { return _sub_Re[var_num]; }
  DenseMatrix<Number> & jacobianBlock(unsigned int ivar, unsigned int jvar) { return _sub_Ke[ivar][jvar]; }

  std::vector<std::pair<unsigned int, unsigned int> > & couplingEntries() { return _cm_entry; }

  const std::vector<std::vector<Real> > & phi() { return _phi; }
  const std::vector<std::vector<RealGradient> > & gradPhi() { return _grad_phi; }
  const std::vector<std::vector<RealTensor> > & secondPhi() { return _second_phi; }

  const std::vector<std::vector<Real> > & phiFace() { return _phi_face; }
  const std::vector<std::vector<RealGradient> > & gradPhiFace() { return _grad_phi_face; }
  const std::vector<std::vector<RealTensor> > & secondPhiFace() { return _second_phi_face; }

protected:
  SystemBase & _sys;
  CouplingMatrix * & _cm;                                       ///< Reference to coupling matrix
  std::vector<std::pair<unsigned int, unsigned int> > _cm_entry; ///< Entries in the coupling matrix
  const DofMap & _dof_map;                                      ///< DOF map
  THREAD_ID _tid;                                               ///< Thread number (id)

  std::vector<std::vector<unsigned int> > _sub_dof_indices;     ///< DOF indices for each variable
  std::vector<DenseVector<Number> > _sub_Re;                    ///< residual contributions for each variable
  std::vector<std::vector<DenseMatrix<Number> > > _sub_Ke;      ///< jacobian contributions

  // Shape function values, gradients. second derivatives
  std::vector<std::vector<Real> > _phi;
  std::vector<std::vector<RealGradient> > _grad_phi;
  std::vector<std::vector<RealTensor> > _second_phi;

  std::vector<std::vector<Real> > _phi_face;
  std::vector<std::vector<RealGradient> > _grad_phi_face;
  std::vector<std::vector<RealTensor> > _second_phi_face;

};

#endif /* ASMBLOCK_H */
