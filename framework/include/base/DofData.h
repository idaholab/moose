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

#ifndef DOFDATA_H
#define DOFDATA_H

//Moose includes
#include "Moose.h"
#include "MooseArray.h"

//libMesh includes
#include "numeric_vector.h"
#include "dense_matrix.h"
#include "dense_subvector.h"

//Forward Declarations
class MooseSystem;
class ElementData;

class DofData
{
public:
  DofData(MooseSystem & moose_system);
  virtual ~DofData();

  void init();

  void reinitRes(int var_num, DenseVector<Number> & Re, unsigned int position, unsigned int num_dofs);

  void reinitKes(int var_num, unsigned int num_dofs);
  void reinitKns(int var_num, unsigned int num_dofs, unsigned int num_n_dofs);

public:
  /**
   * The MooseSystem this object is associated with.
   */
  MooseSystem & _moose_system;

  /**
   * Dof Maps for all the variables.
   */
  std::vector<std::vector<unsigned int> > _var_dof_indices;

  std::vector<unsigned int> _dof_indices;

  /**
   * Holds the current dof numbers for each variable
   */
  std::vector<unsigned int> _aux_var_dofs;

  /**
   * Dof Maps for all the auxiliary variables.
   */
  std::vector<std::vector<unsigned int> > _aux_var_dof_indices;

  /**
   * Current element
   */
  const Elem *_current_elem;

  /**
   * Residual vectors for all variables.
   */
  std::vector<DenseSubVector<Number> * > _var_Res;

  /**
   * Jacobian matrices for all variables.
   */
  std::vector<DenseMatrix<Number> * > _var_Kes;

  /**
   * Jacobian matrices for all variables (used in DG).
   */
  std::vector<DenseMatrix<Number> * > _var_Kns;
};

#endif //DOFDATA_H
