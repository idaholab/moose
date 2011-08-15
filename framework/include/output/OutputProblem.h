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

#ifndef OUTPUTPROBLEM_H
#define OUTPUTPROBLEM_H

#include "MProblem.h"
#include "Output.h"
// libMesh
#include "equation_systems.h"
#include "vector_value.h"
#include "mesh_function.h"

class MooseMesh;

class OutputProblem : public Problem
{
public:
  OutputProblem(MProblem & mproblem, unsigned int refinements);
  virtual ~OutputProblem();

  virtual EquationSystems & es() { return _eq; }
  virtual Problem * parent() { return NULL; }

  // Variables /////
  virtual bool hasVariable(const std::string & var_name) { return false; } // TODO
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name) { return _mproblem.getVariable(tid, var_name); } // TODO

  virtual void subdomainSetup(unsigned int subdomain, THREAD_ID tid) {}
  virtual void subdomainSetupSide(unsigned int subdomain, THREAD_ID tid) {}

  virtual void prepare(const Elem * elem, THREAD_ID tid) {}
  virtual void prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid) {}

  virtual void reinitElem(const Elem * elem, THREAD_ID tid) {}
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid) {}
  virtual void reinitNode(const Node * node, THREAD_ID tid) {}
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid) {}
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid) {}

  // Materials /////
  virtual void reinitMaterials(unsigned int blk_id, THREAD_ID tid) {}
  virtual void reinitMaterialsFace(unsigned int blk_id, unsigned int side, THREAD_ID tid) {}

  // Solve /////
  virtual void init();

  virtual void computeResidual(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, NumericVector<Number> & residual) {}
  virtual void computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> & jacobian) {}

  // Initial conditions /////
  virtual Number initialValue (const Point & p, const Parameters & parameters, const std::string & sys_name, const std::string & var_name) { return 0; }
  virtual Gradient initialGradient (const Point & p, const Parameters & parameters, const std::string & sys_name, const std::string & var_name) { return Gradient(); }

  virtual void initialCondition(EquationSystems & es, const std::string & system_name) {}

  // Postprocessors /////
  virtual void computePostprocessors(ExecFlagType type = EXEC_TIMESTEP) {}
  virtual void outputPostprocessors() {}
  virtual Real & getPostprocessorValue(const std::string & name, THREAD_ID tid = 0) { return _dummy; } // TODO

  // Transient /////
  virtual void transient(bool trans) {}
  virtual bool isTransient() { return _mproblem.isTransient(); }

  virtual Real & time() { return _mproblem.time(); }
  virtual int & timeStep() { return _mproblem.timeStep(); }
  virtual Real & dt() { return _mproblem.dt(); }
  virtual Real & dtOld() { return _mproblem.dtOld(); }

  virtual void copySolutionsBackwards() {}

  // Output system /////

  virtual Output & out() { return _out; }
  virtual void output() {}

protected:
  MProblem & _mproblem;
  MooseMesh _mesh;
  EquationSystems _eq;
  Output _out;
  std::vector<std::vector<MeshFunction *> > _mesh_functions;
  NumericVector<Number> * _serialized_solution;

private:
  Real _dummy;
};

#endif
