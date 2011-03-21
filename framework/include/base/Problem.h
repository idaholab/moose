#ifndef PROBLEM_H_
#define PROBLEM_H_

#include "Output.h"
#include "ParallelUniqueId.h"
#include "InputParameters.h"
#include "MaterialProperty.h"
#include "Function.h"
#include "Mesh.h"
// libMesh
#include "libmesh_common.h"
#include "equation_systems.h"
#include "quadrature.h"
#include "elem.h"
#include "node.h"
#include "nonlinear_implicit_system.h"

namespace Moose
{

class Variable;

class Problem
{
public:
  Problem();
  virtual ~Problem();

  virtual EquationSystems & es() = 0;
//  virtual Mesh & mesh() = 0;

  // Variables /////
  virtual bool hasVariable(const std::string & var_name) = 0;
  virtual Variable & getVariable(THREAD_ID tid, const std::string & var_name) = 0;

  virtual void reinitElem(const Elem * elem, THREAD_ID tid) = 0;
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNode(const Node * node, THREAD_ID tid) = 0;
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid) = 0;

  virtual void subdomainSetup(unsigned int subdomain, THREAD_ID tid) = 0;

  // Materials /////
  virtual void reinitMaterials(unsigned int blk_id, THREAD_ID tid) = 0;
  virtual void reinitMaterialsFace(unsigned int blk_id, unsigned int side, THREAD_ID tid) = 0;

  // Solve /////
  virtual void init() = 0;
  virtual void update() = 0;

  virtual void computeResidual(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, NumericVector<Number> & residual) = 0;
  virtual void computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> & jacobian) = 0;

  // Initial conditions /////
  virtual Number initialValue (const Point & p, const Parameters & parameters, const std::string & sys_name, const std::string & var_name) = 0;
  virtual Gradient initialGradient (const Point & p, const Parameters & parameters, const std::string & sys_name, const std::string & var_name) = 0;

  virtual void initialCondition(EquationSystems & es, const std::string & system_name) = 0;

  // Postprocessors /////
  virtual void computePostprocessors(int pps_type = Moose::PPS_TIMESTEP) = 0;
  virtual void outputPostprocessors() = 0;
  virtual Real & getPostprocessorValue(const std::string & name, THREAD_ID tid = 0) = 0;

  // Transient /////
  virtual void transient(bool trans) = 0;
  virtual bool transient() = 0;

  virtual Real & time() = 0;
  virtual int & timeStep() = 0;
  virtual Real & dt() = 0;
  virtual Real & dtOld() = 0;

  virtual void copySolutionsBackwards() = 0;

  // Output system /////

  virtual Output & out() = 0;
  virtual void output() = 0;
};

}

#endif /* PROBLEM_H_ */
