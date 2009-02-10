#ifndef PHYSICSBASEDPRECONDITIONER_H
#define PHYSICSBASEDPRECONDITIONER_H

//Global includes:
#include<vector>

//libMesh includes:
#include "preconditioner.h"
#include "system.h"

//Forward declarations
class EquationSystems;
class MeshBase;

/**
 * Implements a segregated solve preconditioner.
 */
class PhysicsBasedPreconditioner : public Preconditioner<Number>
{
public:
  
  /**
   *  Constructor. Initializes PhysicsBasedPreconditioner data structures
   */
  PhysicsBasedPreconditioner ();
    
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
  virtual void clear () {}

  /**
   * Initialize data structures if not done so already.
   */
  virtual void init ();

  /**
   * Set the equation_systems object.
   */
  void setEq(EquationSystems & equation_systems){_equation_systems = &equation_systems;}

  /**
   * Set a function pointer for how to evaluate a block of the "Jacobian"
   */
  void setComputeJacobianBlock(void (*compute_jacobian_block) (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar))
  {_compute_jacobian_block = compute_jacobian_block;}

  /**
   * Helper function for copying values associated with variables in vectors from two different systems.
   */
  static void copyVarValues(MeshBase & mesh,
                     const unsigned int from_system, const unsigned int from_var, const NumericVector<Number> & from_vector,
                     const unsigned int to_system, const unsigned int to_var, NumericVector<Number> & to_vector);
  
protected:
  /**
   * The EquationSystems object that all of the systems live in.
   */
  EquationSystems * _equation_systems;

  /**
   * A pointer to the function to call to compute one block of the jacobian.
   */
  void (*_compute_jacobian_block) (const NumericVector<Number>& soln, SparseMatrix<Number>&  jacobian, System& precond_system, unsigned int ivar, unsigned int jvar);

  /**
   * Holds one Preconditioner object per small system to solve.
   */
  std::vector<Preconditioner<Number> *> _preconditioners;
};

inline
PhysicsBasedPreconditioner::PhysicsBasedPreconditioner () :
                           Preconditioner<Number>(),
                           _equation_systems(NULL),
                           _compute_jacobian_block(NULL)
{
}

inline
PhysicsBasedPreconditioner::~PhysicsBasedPreconditioner ()
{
  this->clear ();
}

#endif //PHYSICSBASEDPRECONDITIONER_H
