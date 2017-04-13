/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONTACTMASTER_H
#define CONTACTMASTER_H

// Moose Includes
#include "DiracKernel.h"
#include "PenetrationLocator.h"

enum ContactModel
{
  CM_INVALID,
  CM_FRICTIONLESS,
  CM_GLUED,
  CM_COULOMB,
  CM_COULOMB_MP // Coulomb contact enforced with a "model problem" strategy where slip is computed
                // after the nonlinear solve is converged
};

enum ContactFormulation
{
  CF_INVALID,
  CF_DEFAULT,
  CF_KINEMATIC = CF_DEFAULT,
  CF_PENALTY,
  CF_AUGMENTED_LAGRANGE,
  CF_TANGENTIAL_PENALTY
};

class ContactMaster : public DiracKernel
{
public:
  ContactMaster(const InputParameters & parameters);

  virtual void jacobianSetup();
  virtual void timestepSetup();

  virtual void addPoints();
  void computeContactForce(PenetrationInfo * pinfo);
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  virtual void updateContactSet(bool beginning_of_step = false);

  static ContactFormulation contactFormulation(std::string name);
  static ContactModel contactModel(std::string name);

protected:
  Real nodalArea(PenetrationInfo & pinfo);
  Real getPenalty(PenetrationInfo & pinfo);

  const unsigned int _component;
  const ContactModel _model;
  const ContactFormulation _formulation;
  const bool _normalize_penalty;
  PenetrationLocator & _penetration_locator;

  const Real _penalty;
  const Real _friction_coefficient;
  const Real _tension_release;
  const Real _capture_tolerance;
  bool _updateContactSet;

  NumericVector<Number> & _residual_copy;

  std::map<Point, PenetrationInfo *> _point_to_info;

  const unsigned int _mesh_dimension;

  std::vector<unsigned int> _vars;

  MooseVariable * _nodal_area_var;
  SystemBase & _aux_system;
  const NumericVector<Number> * _aux_solution;
};

ContactModel contactModel(const std::string & the_name);
ContactFormulation contactFormulation(const std::string & the_name);

template <>
InputParameters validParams<ContactMaster>();

#endif // CONTACTMASTER_H
