//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVRhieChowInterpolatorSegregated.h"
#include "INSFVAttributes.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "NS.h"
#include "Assembly.h"
#include "INSFVVelocityVariable.h"
#include "INSFVPressureVariable.h"
#include "PiecewiseByBlockLambdaFunctor.h"
#include "VectorCompositeFunctor.h"
#include "SIMPLENonlinearAssembly.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem_range.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"

#include "NonlinearSystem.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", INSFVRhieChowInterpolatorSegregated);

InputParameters
INSFVRhieChowInterpolatorSegregated::validParams()
{
  auto params = RhieChowInterpolatorBase::validParams();

  params.addClassDescription("Computes H/A and 1/A together with face velocities for segregated "
                             "momentum-pressure equations.");

  // We disable the execution of this, should only provide functions
  // for the SIMPLENonlinearAssembly executioner
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_NONE);
  exec_enum = {EXEC_NONE};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

INSFVRhieChowInterpolatorSegregated::INSFVRhieChowInterpolatorSegregated(
    const InputParameters & params)
  : RhieChowInterpolatorBase(params),
    _HbyA(_moose_mesh, blockIDs(), "HbyA"),
    _Ainv(_moose_mesh, blockIDs(), "Ainv", false),
    _vel(std::make_unique<PiecewiseByBlockLambdaFunctor<ADRealVectorValue>>(
        name(),
        [this](const auto & r, const auto & t) -> ADRealVectorValue
        {
          ADRealVectorValue velocity((*_u)(r, t));
          if (_dim >= 2)
            velocity(1) = (*_v)(r, t);
          if (_dim >= 3)
            velocity(2) = (*_w)(r, t);
          return velocity;
        },
        std::set<ExecFlagType>({EXEC_ALWAYS}),
        _moose_mesh,
        blockIDs())),
    _face_velocity(_moose_mesh, blockIDs(), "face_values")
{
  if (_displaced)
    paramError("use_displaced_mesh",
               "The segregated Rhie-Chow user object does not currently support operation on a "
               "displaced mesh");

  // Register the elemental/face functors which will be queried in the pressure equation
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("Ainv", _Ainv, tid);
    UserObject::_subproblem.addFunctor("HbyA", _HbyA, tid);
  }

  if (_velocity_interp_method == Moose::FV::InterpMethod::Average)
    paramError("velocity_interp_method",
               "Segregated momentum-pressure solvers do not allow average interpolation methods!");

  if (!dynamic_cast<SIMPLENonlinearAssembly *>(getMooseApp().getExecutioner()))
    mooseError(this->name(), " should only be used with a segregated thermal-hydraulics solver!");
}

void
INSFVRhieChowInterpolatorSegregated::linkMomentumSystem(
    std::vector<NonlinearSystemBase *> momentum_systems,
    const std::vector<unsigned int> & momentum_system_numbers,
    const TagID pressure_gradient_tag)
{
  _momentum_systems = momentum_systems;
  _momentum_system_numbers = momentum_system_numbers;
  _pressure_gradient_tag = pressure_gradient_tag;

  _momentum_implicit_systems.clear();
  for (auto & system : _momentum_systems)
    _momentum_implicit_systems.push_back(
        dynamic_cast<NonlinearImplicitSystem *>(&system->system()));
}

void
INSFVRhieChowInterpolatorSegregated::meshChanged()
{
  _HbyA.clear();
  _Ainv.clear();
  _face_velocity.clear();
}

void
INSFVRhieChowInterpolatorSegregated::initialize()
{
  for (const auto & pair : _HbyA)
    _HbyA[pair.first] = 0;

  for (const auto & pair : _Ainv)
    _Ainv[pair.first] = 0;
}

void
INSFVRhieChowInterpolatorSegregated::initFaceVelocities()
{
  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
    {
      // On internal face we do a regular interpoaltion with geometric weights
      if (_u->isInternalFace(*fi))
      {
        const Moose::FaceArg face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};

        _face_velocity[fi->id()] = MetaPhysicL::raw_value((*_vel)(face, Moose::currentState()));
      }
      // On the boundary, we just take the boundary values
      else
      {
        const Elem * const boundary_elem =
            hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();

        const Moose::FaceArg boundary_face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};

        _face_velocity[fi->id()] =
            MetaPhysicL::raw_value((*_vel)(boundary_face, Moose::currentState()));
      }
    }
  }
}

VectorValue<ADReal>
INSFVRhieChowInterpolatorSegregated::getVelocity(const Moose::FV::InterpMethod m,
                                                 const FaceInfo & fi,
                                                 const Moose::StateArg & /*time*/,
                                                 const THREAD_ID /*tid*/,
                                                 const bool /*subtract_mesh_velocity*/) const
{
  if (m != Moose::FV::InterpMethod::RhieChow)
    mooseError("Segregated solution algorithms only support Rhie-Chow interpolation!");
  return _face_velocity.evaluate(&fi);
}

void
INSFVRhieChowInterpolatorSegregated::computeFaceVelocity()
{
  const auto time_arg = Moose::currentState();

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
    {
      // On internal face we just use the interpolated H/A and the pressure face gradient
      // So u_f = -(H/A)_f - (1/A)_f*grad(p)_f
      // Notice the (-) sign on H/A which is because we use the Jacobian/Residual
      // computations and we get -H instead of H.
      if (_u->isInternalFace(*fi))
      {
        const Moose::FaceArg face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};

        RealVectorValue Ainv;
        RealVectorValue HbyA = MetaPhysicL::raw_value(_HbyA(face, time_arg));

        interpolate(Moose::FV::InterpMethod::Average,
                    Ainv,
                    _Ainv(makeElemArg(fi->elemPtr()), time_arg),
                    _Ainv(makeElemArg(fi->neighborPtr()), time_arg),
                    *fi,
                    true);

        RealVectorValue grad_p = MetaPhysicL::raw_value(_p->gradient(face, time_arg));
        for (const auto comp_index : make_range(_dim))
          _face_velocity[fi->id()](comp_index) =
              -HbyA(comp_index) - Ainv(comp_index) * grad_p(comp_index);
      }
      else
      {
        const Elem * const boundary_elem =
            hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();
        const Moose::FaceArg boundary_face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};

        // If we have a dirichlet boundary conditions, this sill give us the exact value of the
        // velocity on the face as expected (see populateHbyA())
        if (_u->isDirichletBoundaryFace(*fi, boundary_elem, time_arg))
          _face_velocity[fi->id()] = -MetaPhysicL::raw_value(_HbyA(boundary_face, time_arg));
        else
        {
          const RealVectorValue & Ainv = MetaPhysicL::raw_value(_Ainv(boundary_face, time_arg));
          const RealVectorValue & HbyA = MetaPhysicL::raw_value(_HbyA(boundary_face, time_arg));
          const RealVectorValue & grad_p =
              MetaPhysicL::raw_value(_p->gradient(boundary_face, time_arg));
          for (const auto comp_index : make_range(_dim))
            _face_velocity[fi->id()](comp_index) =
                -HbyA(comp_index) - Ainv(comp_index) * grad_p(comp_index);
        }
      }
    }
  }
}

void
INSFVRhieChowInterpolatorSegregated::computeCellVelocity()
{
  std::vector<unsigned int> var_nums = {_momentum_implicit_systems[0]->variable_number(_u->name())};
  if (_v)
    var_nums.push_back(_momentum_implicit_systems[1]->variable_number(_v->name()));
  if (_w)
    var_nums.push_back(_momentum_implicit_systems[2]->variable_number(_w->name()));

  const auto time_arg = Moose::currentState();

  for (auto & elem :
       as_range(_mesh.active_local_elements_begin(), _mesh.active_local_elements_end()))
  {
    if (hasBlocks(elem->subdomain_id()))
    {
      const auto elem_arg = makeElemArg(elem);
      const RealVectorValue Ainv = _Ainv(elem_arg, time_arg);
      const RealVectorValue & grad_p = raw_value(_p->gradient(elem_arg, time_arg));

      for (auto comp_index : make_range(_dim))
      {
        // If we are doing segregated momentum components we need to access different vector
        // components otherwise everything is in the same vector (with different variable names)
        const unsigned int system_number = _momentum_implicit_systems[comp_index]->number();
        const auto index = elem->dof_number(system_number, var_nums[comp_index], 0);

        // We set the dof value in the solution vector the same logic applies:
        // u_C = -(H/A)_C - (1/A)_C*grad(p)_C where C is the cell index
        _momentum_implicit_systems[comp_index]->solution->set(
            index, -(*_HbyA_raw[comp_index])(index)-Ainv(comp_index) * grad_p(comp_index));
      }
    }
  }

  for (auto system_i : index_range(_momentum_implicit_systems))
  {
    _momentum_implicit_systems[system_i]->solution->close();
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }
}

void
INSFVRhieChowInterpolatorSegregated::populateHbyA(
    const std::vector<std::unique_ptr<NumericVector<Number>>> & raw_hbya,
    const std::vector<unsigned int> & var_nums)
{
  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
    {
      // If we are on an internal face, we just interpolate the values to the faces.
      // Otherwise, depending on the boundary type, we take the velocity value or
      // extrapolated HbyA values.
      if (_u->isInternalFace(*fi))
      {
        const Elem * elem = fi->elemPtr();
        const Elem * neighbor = fi->neighborPtr();
        for (auto comp_index : make_range(_dim))
        {
          unsigned int system_number = _momentum_implicit_systems[comp_index]->number();
          const auto dof_index_elem = elem->dof_number(system_number, var_nums[comp_index], 0);
          const auto dof_index_neighbor =
              neighbor->dof_number(system_number, var_nums[comp_index], 0);

          interpolate(Moose::FV::InterpMethod::Average,
                      _HbyA[fi->id()](comp_index),
                      (*raw_hbya[comp_index])(dof_index_elem),
                      (*raw_hbya[comp_index])(dof_index_neighbor),
                      *fi,
                      true);
        }
      }
      else
      {
        const Elem * const boundary_elem =
            hasBlocks(fi->elemPtr()->subdomain_id()) ? fi->elemPtr() : fi->neighborPtr();

        const Moose::FaceArg boundary_face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};

        if (_u->isDirichletBoundaryFace(*fi, boundary_elem, Moose::currentState()))
          _HbyA[fi->id()] = -MetaPhysicL::raw_value((*_vel)(boundary_face, Moose::currentState()));
        else
          for (const auto comp_index : make_range(_dim))
          {
            unsigned int system_number = _momentum_implicit_systems[comp_index]->number();
            const auto dof_index_elem =
                boundary_elem->dof_number(system_number, var_nums[comp_index], 0);
            _HbyA[fi->id()](comp_index) = (*raw_hbya[comp_index])(dof_index_elem);
          }
      }
    }
  }
}

void
INSFVRhieChowInterpolatorSegregated::computeHbyA(bool verbose)
{
  if (verbose)
  {
    _console << "************************************" << std::endl;
    _console << "Computing HbyA" << std::endl;
    _console << "************************************" << std::endl;
  }
  mooseAssert(_momentum_implicit_systems.size() && _momentum_implicit_systems[0],
              "The momentum system shall be linked before calling this function!");

  NonlinearImplicitSystem * momentum_system = _momentum_implicit_systems[0];

  std::vector<unsigned int> var_nums = {_momentum_implicit_systems[0]->variable_number(_u->name())};
  if (_v)
    var_nums.push_back(_momentum_implicit_systems[1]->variable_number(_v->name()));
  if (_w)
    var_nums.push_back(_momentum_implicit_systems[2]->variable_number(_w->name()));

  _HbyA_raw.clear();
  for (auto system_i : index_range(_momentum_systems))
  {
    _fe_problem.setCurrentNonlinearSystem(_momentum_system_numbers[system_i]);

    momentum_system = _momentum_implicit_systems[system_i];

    NumericVector<Number> & rhs = *(momentum_system->rhs);
    NumericVector<Number> & current_local_solution = *(momentum_system->current_local_solution);
    NumericVector<Number> & solution = *(momentum_system->solution);
    PetscMatrix<Number> * mmat = dynamic_cast<PetscMatrix<Number> *>(momentum_system->matrix);
    mooseAssert(mmat,
                "The matrices used in the segregated INSFVRhieChow objects need to be convertable "
                "to PetscMAtrix!");

    if (verbose)
    {
      _console << "Matrix in rc object" << std::endl;
      mmat->print();
    }

    auto Ainv = current_local_solution.zero_clone();
    PetscVector<Number> * Ainv_petsc = dynamic_cast<PetscVector<Number> *>(Ainv.get());

    mmat->get_diagonal(*Ainv_petsc);

    auto working_vector = momentum_system->current_local_solution->zero_clone();
    PetscVector<Number> * working_vector_petsc =
        dynamic_cast<PetscVector<Number> *>(working_vector.get());
    mooseAssert(working_vector_petsc,
                "The vectors used in the segregated INSFVRhieChow objects need to be convertable "
                "to PetscVectors!");

    *working_vector_petsc = 1.0;
    Ainv_petsc->pointwise_divide(*working_vector_petsc, *Ainv_petsc);

    _HbyA_raw.push_back(current_local_solution.zero_clone());
    NumericVector<Number> & HbyA = *(_HbyA_raw.back());

    if (verbose)
    {
      _console << "Velocity solution in H(u)" << std::endl;
      solution.print();
    }

    // We fill the 1/A functor
    auto active_local_begin =
        _mesh.evaluable_elements_begin(momentum_system->get_dof_map(), var_nums[system_i]);
    auto active_local_end =
        _mesh.evaluable_elements_end(momentum_system->get_dof_map(), var_nums[system_i]);

    const auto & state = Moose::currentState();
    for (auto it = active_local_begin; it != active_local_end; ++it)
    {
      const Elem * elem = *it;
      if (this->hasBlocks(elem->subdomain_id()))
      {
        const Moose::ElemArg & elem_arg = makeElemArg(elem);
        Real coord_multiplier;
        const auto coord_type = _fe_problem.getCoordSystem(elem->subdomain_id());
        const unsigned int rz_radial_coord =
            Moose::COORD_RZ ? _fe_problem.getAxisymmetricRadialCoord() : libMesh::invalid_uint;

        MooseMeshUtils::coordTransformFactor(
            elem->vertex_average(), coord_multiplier, coord_type, rz_radial_coord);

        const Real volume = _moose_mesh.elemInfo(elem->id()).volume();
        const auto dof_index = elem->dof_number(momentum_system->number(), var_nums[system_i], 0);
        _Ainv[elem->id()](system_i) = MetaPhysicL::raw_value(epsilon(_tid)(elem_arg, state)) *
                                      (*Ainv_petsc)(dof_index)*volume * coord_multiplier;
      }
    }

    // Now we set the diagonal of our system matrix to 0 so we can create H*u
    // TODO: Add a function for this in libmesh
    *working_vector_petsc = 0.0;
    LibmeshPetscCall(MatDiagonalSet(mmat->mat(), working_vector_petsc->vec(), INSERT_VALUES));

    if (verbose)
    {
      _console << "H" << std::endl;
      mmat->print();
    }

    // We need to subtract the contribution of the pressure gradient from the residual (right hand
    // side). We plug working_vector=0 in here to get the right hand side contribution
    _fe_problem.computeResidualTag(*working_vector, HbyA, _pressure_gradient_tag);

    // Now we reorganize the association between tags and vectors to make sure
    // that the next solve is perfect
    _momentum_systems[system_i]->associateVectorToTag(
        momentum_system->get_vector(_fe_problem.vectorTagName(0)), 0);
    _momentum_systems[system_i]->associateVectorToTag(
        momentum_system->get_vector(_fe_problem.vectorTagName(_pressure_gradient_tag)),
        _pressure_gradient_tag);
    _momentum_systems[system_i]->setSolution(current_local_solution);

    if (verbose)
    {
      _console << "total RHS" << std::endl;
      rhs.print();
      _console << "pressure RHS" << std::endl;
      HbyA.print();
    }

    // We correct the right hand side to exclude the pressure contribution
    HbyA.scale(-1.0);
    HbyA.add(-1.0, rhs);

    if (verbose)
    {
      _console << "H RHS" << std::endl;
      HbyA.print();
    }

    // Create H(u)
    mmat->vector_mult(*working_vector_petsc, solution);

    if (verbose)
    {
      _console << " H(u)" << std::endl;
      working_vector_petsc->print();
    }

    // Create H(u) - RHS
    HbyA.add(*working_vector_petsc);

    if (verbose)
    {
      _console << " H(u)-rhs-relaxation_source" << std::endl;
      HbyA.print();
    }

    // Create 1/A*(H(u)-RHS)
    HbyA.pointwise_mult(HbyA, *Ainv);

    if (verbose)
    {
      _console << " (H(u)-rhs)/A" << std::endl;
      HbyA.print();
    }
  }

  populateHbyA(_HbyA_raw, var_nums);

  if (verbose)
  {
    _console << "************************************" << std::endl;
    _console << "DONE Computing HbyA " << std::endl;
    _console << "************************************" << std::endl;
  }
}
