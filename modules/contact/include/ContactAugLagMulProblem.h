/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONTACTAUGLAGMULPROBLEM_H
#define CONTACTAUGLAGMULPROBLEM_H

#include "ReferenceResidualProblem.h"
#include "NodeFaceConstraint.h"
#include "ContactMaster.h"
#include "MechanicalContactConstraint.h"

class ContactAugLagMulProblem;

template <>
InputParameters validParams<ContactAugLagMulProblem>();

/**
 * FEProblemBase derived class for frictional contact-specific callbacks
 */
class ContactAugLagMulProblem : public ReferenceResidualProblem
{
public:
  ContactAugLagMulProblem(const InputParameters & params);
  virtual ~ContactAugLagMulProblem() {}

  struct InteractionParams;

  virtual void initialSetup();
  virtual void timestepSetup();

  void updateContactReferenceResidual();

  virtual MooseNonlinearConvergenceReason checkNonlinearConvergence(std::string & msg,
                                                                    const PetscInt it,
                                                                    const Real xnorm,
                                                                    const Real snorm,
                                                                    const Real fnorm,
                                                                    const Real rtol,
                                                                    const Real stol,
                                                                    const Real abstol,
                                                                    const PetscInt nfuncs,
                                                                    const PetscInt max_funcs,
                                                                    const Real ref_resid,
                                                                    const Real div_threshold);

protected:
  std::map<std::pair<int, int>, InteractionParams> _interaction_params;
  NonlinearVariableName _disp_x;
  NonlinearVariableName _disp_y;
  NonlinearVariableName _disp_z;
  // AuxVariableName _residual_x;
  // AuxVariableName _residual_y;
  // AuxVariableName _residual_z;

  std::vector<std::string> _contactRefResidVarNames;
  std::vector<unsigned int> _contactRefResidVarIndices;
  const Real _penalty;
  const bool _normalize_penalty;
  //  MooseVariable * _nodal_area_var;
  //  SystemBase & _aux_system;
  //  const NumericVector<Number> * _aux_solution;

  Real _refResidContact;

  bool _do_lagmul_update;
  int _num_lagmul_iterations;
  int _min_lagmul_iters;
  int _max_lagmul_iters;
  int _lagmul_updates_per_iter;
  Real _contact_lagmul_tol_factor;
  int _num_nl_its_since_contact_update;
  int _num_contact_nodes;

  /// Convenient typedef for frequently used iterator
  typedef std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator
      PLIterator;
};

struct ContactAugLagMulProblem::InteractionParams
{
  Real _friction_coefficient;
  Real _lagmul_factor;
  Real _lagmul_too_far_factor;
};

#endif /* CONTACTAUGLAGMULPROBLEM_H */
