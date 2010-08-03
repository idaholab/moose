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
};

#endif //DOFDATA_H
