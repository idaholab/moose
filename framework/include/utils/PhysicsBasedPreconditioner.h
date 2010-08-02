#ifndef PHYSICSBASEDPRECONDITIONER_H
#define PHYSICSBASEDPRECONDITIONER_H

//Moose Includes
#include "MooseSystem.h"

//Global includes:
#include<vector>

//libMesh includes:
#include "preconditioner.h"
#include "system.h"
#include "nonlinear_implicit_system.h"

//Forward declarations
namespace libMesh
{
  class EquationSystems;
  class MeshBase;
}

/**
 * Implements a segregated solve preconditioner.
 */
class PhysicsBasedPreconditioner : public Preconditioner<Number>
{
public:
  
  /**
   *  Constructor. Initializes PhysicsBasedPreconditioner data structures
   */
  PhysicsBasedPreconditioner (MooseSystem & moose_system);
    
  /**
   * Destructor.
   */
  virtual ~PhysicsBasedPreconditioner ();
    

  /**
   * Computes the preconditioned vector "y" based on input "x".
   * Usually by solving Py=x to get the action of P^-1 x.
   */
  virtual void apply(const NumericVector<Number> & x, NumericVector<Number> & y);
  
  /**
   * Release all memory and clear data structures.
   */
  virtual void clear ();

  /**
   * Initialize data structures if not done so already.
   */
  virtual void init ();
  
  /**
   * Set the order the block rows are solved for.  If not set then the solve happens in the order
   * the variables were added to the NonlinearSystem.
   */
  void setSolveOrder(std::vector<unsigned int> solve_order);
  
  /**
   * Sets the preconditioner type to use for each solve.
   * These are in the same order as the variable numbers in the
   * System.
   *
   * If you don't call this function the default is to use AMG for
   * everything.
   * 
   */
  void setPreconditionerType(std::vector<PreconditionerType> pre_type);
  
  /**
   * Set which off diagonal blocks need to get computed and used by the preconditioner.
   * off_diag[number] should be a vector of the off diagonal blocks needed by var_num = number.
   * The blocks are specified by the coupled var_nums. So to compute the off diagonal block for
   * variable 1 corresponding to variable 3 off_diag[1] should have 3 in it somewhere. The
   * order of the off diagnal blocks doesn't matter.
   */
  void setOffDiagBlocks(std::vector<std::vector<unsigned int> > off_diag);
  
  /**
   * Helper function for copying values associated with variables in vectors from two different systems.
   */
  static void copyVarValues(MeshBase & mesh,
                     const unsigned int from_system, const unsigned int from_var, const NumericVector<Number> & from_vector,
                     const unsigned int to_system, const unsigned int to_var, NumericVector<Number> & to_vector);
  
protected:
  /**
   * The MooseSystem this PBP is associated with.
   */
  MooseSystem & _moose_system;
  
  /**
   * The EquationSystems object that all of the systems live in.
   */
  EquationSystems * _equation_systems;

  /**
   * Holds one Preconditioner object per small system to solve.
   */
  std::vector<Preconditioner<Number> *> _preconditioners;

  /**
   * Holds the order the blocks are solved for.
   */
  std::vector<unsigned int> _solve_order;

  /**
   * Which preconditioner to use for each solve.
   */
  std::vector<PreconditionerType> _pre_type;

  /**
   * Holds which off diagonal blocks to compute.
   */
  std::vector<std::vector<unsigned int> > _off_diag;

  /**
   * Holds pointers to the off-diagonal matrices.
   * This is in the same order as _off_diag.
   *
   * This is really just for convenience so we don't have
   * to keep looking this thing up through it's name.
   */
  std::vector<std::vector<SparseMatrix<Number> *> > _off_diag_mats;
};

inline
PhysicsBasedPreconditioner::PhysicsBasedPreconditioner (MooseSystem & moose_system)
  :_moose_system(moose_system),
   Preconditioner<Number>(),
   _equation_systems(moose_system.getEquationSystems())
{
}

inline
PhysicsBasedPreconditioner::~PhysicsBasedPreconditioner ()
{
  this->clear ();

  std::vector<Preconditioner<Number> *>::iterator it;
  for (it = _preconditioners.begin(); it != _preconditioners.end(); ++it)
    delete *it;
}

#endif //PHYSICSBASEDPRECONDITIONER_H
