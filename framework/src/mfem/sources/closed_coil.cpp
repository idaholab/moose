#include "closed_coil.hpp"

#include <utility>

namespace hephaestus
{

// Pushes an element into a vector if the vector does not yet contain that same
// element
template <typename T>
void
pushIfUnique(std::vector<T> & vec, const T el)
{

  bool verify = true;

  for (auto e : vec)
  {
    if (e == el)
      verify = false;
  }

  if (verify == true)
    vec.push_back(el);
}

// Base class methods

ClosedCoilSolver::ClosedCoilSolver(std::string source_efield_gf_name,
                                   std::string hcurl_fespace_name,
                                   std::string h1_fespace_name,
                                   std::string i_coef_name,
                                   std::string cond_coef_name,
                                   mfem::Array<int> coil_dom,
                                   const int electrode_face,
                                   bool electric_field_transfer,
                                   std::string source_jfield_gf_name,
                                   hephaestus::InputParameters solver_options)
  : _hcurl_fespace_name(std::move(hcurl_fespace_name)),
    _h1_fespace_name(std::move(h1_fespace_name)),
    _source_electric_field_name(std::move(source_efield_gf_name)),
    _source_current_density_name(std::move(source_jfield_gf_name)),
    _i_coef_name(std::move(i_coef_name)),
    _cond_coef_name(std::move(cond_coef_name)),
    _electric_field_transfer(std::move(electric_field_transfer)),
    _coil_domains(std::move(coil_dom))
{
  _elec_attrs.first = electrode_face;
}

void
ClosedCoilSolver::Init(hephaestus::GridFunctions & gridfunctions,
                       const hephaestus::FESpaces & fespaces,
                       hephaestus::BCMap & bc_map,
                       hephaestus::Coefficients & coefficients)
{
  // Retrieving the parent FE space and mesh
  _h_curl_fe_space_parent = fespaces.Get(_hcurl_fespace_name);

  // Setting the ccs coefs which will be altered later
  _ccs_coefs = coefficients;

  _mesh_parent = _h_curl_fe_space_parent->GetParMesh();
  _order_hcurl = _h_curl_fe_space_parent->FEColl()->GetOrder();
  _order_h1 = _order_hcurl;

  // Optional FE Spaces and parameters
  if (!fespaces.Has(_h1_fespace_name))
  {
    logger.info("{} not found in fespaces when creating {}. Creating from mesh.",
                _h1_fespace_name,
                typeid(this).name());

    // Need to free this memory after use. FEC not freed by ParFiniteElementSpace destructor!
    _h1_fe_space_parent_fec =
        std::make_unique<mfem::H1_FECollection>(_order_h1, _mesh_parent->Dimension());

    _h1_fe_space_parent =
        std::make_shared<mfem::ParFiniteElementSpace>(_mesh_parent, _h1_fe_space_parent_fec.get());
  }
  else
  {
    _h1_fe_space_parent = fespaces.GetShared(_h1_fespace_name);
  }

  if (!gridfunctions.Has(_source_electric_field_name))
  {
    logger.info("{} not found in gridfunctions when creating {}. Creating new GridFunction.",
                _source_electric_field_name,
                typeid(this).name());
    _source_electric_field = std::make_shared<mfem::ParGridFunction>(_h_curl_fe_space_parent);
  }
  else
  {
    _source_electric_field = gridfunctions.GetShared(_source_electric_field_name);
  }

  if (_source_electric_field->ParFESpace()->FEColl()->GetContType() !=
      mfem::FiniteElementCollection::TANGENTIAL)
  {
    mfem::mfem_error("Electric field GridFunction must be of HCurl type.");
  }

  if (!_source_current_density_name.empty())
  {
    _source_current_density = gridfunctions.GetShared(_source_current_density_name);
    if (_source_current_density == nullptr)
    {
      const std::string error_message = _source_current_density_name +
                                        " not found in gridfunctions when "
                                        "creating OpenCoilSolver\n";
      mfem::mfem_error(error_message.c_str());
    }
    else if (_source_current_density->ParFESpace()->FEColl()->GetContType() !=
             mfem::FiniteElementCollection::NORMAL)
    {
      mfem::mfem_error("Current density GridFunction must be of HDiv type.");
    }
  }

  if (!coefficients._scalars.Has(_i_coef_name))
  {
    logger.info("{} not found in coefficients when creating {}. Assuming unit current.",
                _i_coef_name,
                typeid(this).name());
    _itotal = std::make_shared<mfem::ConstantCoefficient>(1.0);
  }
  else
  {
    _itotal = coefficients._scalars.GetShared(_i_coef_name);
  }

  if (!coefficients._scalars.Has(_cond_coef_name))
  {
    logger.info("{} not found in coefficients when creating {}. Assuming unit conductivity.",
                _cond_coef_name,
                typeid(this).name());
    logger.warn(
        "GradPhi field undefined. The GridFunction associated with it will be set to zero.");
    _sigma = std::make_shared<mfem::ConstantCoefficient>(1.0);

    _electric_field_transfer = false;
  }
  else
  {
    _sigma = coefficients._scalars.GetShared(_cond_coef_name);
  }

  if (_final_lf == nullptr)
  {
    _final_lf = std::make_unique<mfem::ParLinearForm>(_h_curl_fe_space_parent);
    *_final_lf = 0.0;
  }

  MakeWedge();
  PrepareCoilSubmesh();
  SolveTransition();
  SolveCoil();
  RestoreAttributes();
}

void
ClosedCoilSolver::Apply(mfem::ParLinearForm * lf)
{
  // The transformation and integration points themselves are not relevant, it's
  // just so we can call Eval
  mfem::ElementTransformation * tr = _mesh_parent->GetElementTransformation(0);
  const mfem::IntegrationPoint & ip =
      mfem::IntRules.Get(_h_curl_fe_space_parent->GetFE(0)->GetGeomType(), 1).IntPoint(0);

  double i = _itotal->Eval(*tr, ip);
  lf->Add(i, *_final_lf);

  *_source_electric_field = 0.0;
  if (_electric_field_transfer)
  {
    _source_electric_field->Add(i, *_electric_field_t_parent);
  }

  if (_source_current_density)
  {
    hephaestus::GridFunctions aux_gf;
    aux_gf.Register("source_electric_field", _source_electric_field);
    aux_gf.Register("source_current_density", _source_current_density);

    hephaestus::Coefficients aux_coef;
    aux_coef._scalars.Register("electrical_conductivity", _sigma);

    hephaestus::ScaledVectorGridFunctionAux current_density_auxsolver(
        "source_electric_field", "source_current_density", "electrical_conductivity", 1.0);
    current_density_auxsolver.Init(aux_gf, aux_coef);
    current_density_auxsolver.Solve();
  }
}

void
ClosedCoilSolver::SubtractSource(mfem::ParGridFunction * gf)
{
}

// ClosedCoilSolver main methods

void
ClosedCoilSolver::MakeWedge()
{
  std::vector<int> bdr_els;

  // First we save the current domain attributes so they may be restored later
  for (int e = 0; e < _mesh_parent->GetNE(); ++e)
    _old_dom_attrs.push_back(_mesh_parent->GetAttribute(e));

  _new_domain_attr = _mesh_parent->attributes.Max() + 1;

  _elec_attrs.second = _mesh_parent->bdr_attributes.Max() + 1;

  // Now we need to find the electrode boundary
  for (int i = 0; i < _mesh_parent->GetNBE(); ++i)
  {
    if (_mesh_parent->GetBdrAttribute(i) == _elec_attrs.first)
    {
      bdr_els.push_back(i);
    }
  }

  Plane3D plane;

  if (bdr_els.size() > 0)
  {
    plane.Make3DPlane(_mesh_parent, _mesh_parent->GetBdrElementFaceIndex(bdr_els[0]));
  }

  std::vector<int> elec_vtx;
  // Create a vector containing all of the vertices on the electrode
  for (auto b_fc : bdr_els)
  {

    mfem::Array<int> face_vtx;
    _mesh_parent->GetFaceVertices(_mesh_parent->GetBdrElementFaceIndex(b_fc), face_vtx);

    for (auto v : face_vtx)
      pushIfUnique(elec_vtx, v);
  }

  // Now we need to find all elements in the mesh that touch, on at least one
  // vertex, the electrode face. If they do touch the vertex, are on one side of
  // the electrode, and belong to the coil domain, we add them to our wedge

  std::vector<int> wedge_els;

  for (int e = 0; e < _mesh_parent->GetNE(); ++e)
  {

    if (!IsInDomain(e, _coil_domains, _mesh_parent) ||
        plane.Side(ElementCentre(e, _mesh_parent)) == 1)
      continue;

    mfem::Array<int> elem_vtx;
    _mesh_parent->GetElementVertices(e, elem_vtx);

    for (auto v1 : elem_vtx)
    {
      for (auto v2 : elec_vtx)
      {
        if (v1 == v2)
        {
          pushIfUnique(wedge_els, e);
        }
      }
    }
  }

  AddWedgeToPWCoefs(wedge_els);

  // Now we set the second electrode boundary attribute. Start with a list of
  // all the faces of the wedge elements and eliminate mesh and coil boundaries,
  // the first electrode, and faces between wedge elements

  std::vector<int> wedge_faces;
  mfem::Array<int> el_faces;
  mfem::Array<int> ori;

  for (auto e : wedge_els)
  {
    _mesh_parent->GetElementFaces(e, el_faces, ori);
    for (auto f : el_faces)
      pushIfUnique(wedge_faces, f);
  }

  for (auto wf : wedge_faces)
  {

    int e1, e2;
    _mesh_parent->GetFaceElements(wf, &e1, &e2);

    // If the face is a coil boundary
    if (!(IsInDomain(e1, _coil_domains, _mesh_parent) &&
          IsInDomain(e2, _coil_domains, _mesh_parent)))
    {
      continue;
    }

    // If the face is not true interior
    if (!(_mesh_parent->FaceIsInterior(wf) ||
          (_mesh_parent->GetFaceInformation(wf).tag == mfem::Mesh::FaceInfoTag::SharedConforming ||
           _mesh_parent->GetFaceInformation(wf).tag ==
               mfem::Mesh::FaceInfoTag::SharedSlaveNonconforming)))
    {
      continue;
    }

    // If the face is shared between two elements internal to the wedge
    bool test1 = false;
    bool test2 = false;
    for (auto e : wedge_els)
    {
      if (e == e1)
        test1 = true;
      if (e == e2)
        test2 = true;
    }

    if (test1 && test2)
      continue;

    // If the face is part of the first electrode
    test1 = false;
    for (auto b_fc : bdr_els)
    {
      if (wf == _mesh_parent->GetBdrElementFaceIndex(b_fc))
      {
        test1 = true;
        break;
      }
    }
    if (test1)
      continue;

    // At last, if the face is none of these things, it must be our second
    // electrode
    auto * new_elem = _mesh_parent->GetFace(wf)->Duplicate(_mesh_parent);
    new_elem->SetAttribute(_elec_attrs.second);
    _mesh_parent->AddBdrElement(new_elem);
  }

  // Only after this do we set the domain attributes
  for (auto e : wedge_els)
    _mesh_parent->SetAttribute(e, _new_domain_attr);

  _transition_domain.Append(_new_domain_attr);
  _coil_domains.Append(_new_domain_attr);

  _mesh_parent->FinalizeTopology();
  _mesh_parent->Finalize();
  _mesh_parent->SetAttributes();
}

void
ClosedCoilSolver::AddWedgeToPWCoefs(std::vector<int> & wedge_els)
{

  // First, define what in which of the old subdomains the wedge elements of the new subdomain lie
  std::vector<hephaestus::Subdomain> subdomains = _ccs_coefs._subdomains;
  hephaestus::Subdomain new_domain("wedge", _new_domain_attr);
  int wedge_old_att = -1;

  int ref_rank = -1;
  bool has_els = (bool)wedge_els.size();

  // Gather all has_els info from the processes to determine the lowest rank that contains wedge
  // elements
  bool * all_has_els = (bool *)malloc(sizeof(bool) * mfem::Mpi::WorldSize());
  MPI_Allgather(&has_els, 1, MPI_CXX_BOOL, all_has_els, 1, MPI_CXX_BOOL, MPI_COMM_WORLD);

  for (int i = 0; i < mfem::Mpi::WorldSize(); ++i)
  {
    if (all_has_els[i] == true)
    {
      ref_rank = i;
      break;
    }
  }
  if (ref_rank == -1)
    mfem::mfem_error("ClosedCoilSolver wedge has size zero!");

  free(all_has_els);

  if (mfem::Mpi::WorldRank() == ref_rank)
    wedge_old_att = _mesh_parent->GetAttribute(wedge_els[0]);

  MPI_Bcast(&wedge_old_att, 1, MPI_INT, ref_rank, MPI_COMM_WORLD);

  // Now we check in which of the old subdomains the wedge lies
  int sd_wedge = -1;

  for (int i = 0; i < subdomains.size(); ++i)
  {
    if (subdomains[i]._id == wedge_old_att)
    {
      sd_wedge = i;
      break;
    }
  }

  // If we are dealing with piecewise-defined coefficients, we need to
  // add the new wedge domain to the PW coefficients. First we decide whether there are scalar
  // and/or vector PW coefficients
  std::vector<std::string> pw_scalar_coefs;
  for (const auto & coef : _ccs_coefs._scalars)
  {
    std::shared_ptr<mfem::PWCoefficient> id_test =
        std::dynamic_pointer_cast<mfem::PWCoefficient>(coef.second);
    if (id_test != nullptr)
      pw_scalar_coefs.push_back(coef.first);
  }

  std::vector<std::string> pw_vec_coefs;
  for (const auto & coef : _ccs_coefs._vectors)
  {
    std::shared_ptr<mfem::PWVectorCoefficient> id_test =
        std::dynamic_pointer_cast<mfem::PWVectorCoefficient>(coef.second);
    if (id_test != nullptr)
      pw_vec_coefs.push_back(coef.first);
  }

  for (const auto & name : pw_scalar_coefs)
  {
    if (sd_wedge != -1)
    {
      new_domain._scalar_coefficients.Register(
          name, subdomains[sd_wedge]._scalar_coefficients.GetShared(name));
      _ccs_coefs._scalars.Deregister(name);
    }
    else
    {
      logger.warn("Could not find old attribute for wedge defined in the coefficient subdomains! "
                  "Setting null coefficient for {} in wedge",
                  name);
      new_domain._scalar_coefficients.Register(name,
                                               std::make_shared<mfem::ConstantCoefficient>(0.0));
      _ccs_coefs._scalars.Deregister(name);
    }
  }

  for (const auto & name : pw_vec_coefs)
  {
    if (sd_wedge != -1)
    {
      new_domain._vector_coefficients.Register(
          name, subdomains[sd_wedge]._vector_coefficients.GetShared(name));
      _ccs_coefs._vectors.Deregister(name);
    }
    else
    {
      logger.warn("Could not find old attribute for wedge defined in the coefficient subdomains! "
                  "Setting null coefficient for {} in wedge",
                  name);
      mfem::Vector zero_vec(_mesh_parent->Dimension());
      zero_vec = 0.0;
      new_domain._vector_coefficients.Register(
          name, std::make_shared<mfem::VectorConstantCoefficient>(zero_vec));
      _ccs_coefs._vectors.Deregister(name);
    }
  }

  subdomains.push_back(new_domain);
  _ccs_coefs._subdomains = subdomains;
  _ccs_coefs.AddGlobalCoefficientsFromSubdomains();
  _sigma = _ccs_coefs._scalars.GetShared(_cond_coef_name);
}

void
ClosedCoilSolver::PrepareCoilSubmesh()
{
  _mesh_coil = std::make_unique<mfem::ParSubMesh>(
      mfem::ParSubMesh::CreateFromDomain(*_mesh_parent, _coil_domains));

  _electric_field_aux_coil_fec =
      std::make_unique<mfem::ND_FECollection>(_order_hcurl, _mesh_coil->Dimension());

  _electric_field_aux_coil_fes = std::make_unique<mfem::ParFiniteElementSpace>(
      _mesh_coil.get(), _electric_field_aux_coil_fec.get());

  _electric_field_aux_coil =
      std::make_unique<mfem::ParGridFunction>(_electric_field_aux_coil_fes.get());
  *_electric_field_aux_coil = 0.0;

  _h1_fe_space_coil_fec =
      std::make_unique<mfem::H1_FECollection>(_order_h1, _mesh_coil->Dimension());

  _h1_fe_space_coil =
      std::make_unique<mfem::ParFiniteElementSpace>(_mesh_coil.get(), _h1_fe_space_coil_fec.get());

  _v_coil = std::make_unique<mfem::ParGridFunction>(_h1_fe_space_coil.get());
  *_v_coil = 0.0;

  _mesh_t = std::make_unique<mfem::ParSubMesh>(
      mfem::ParSubMesh::CreateFromDomain(*_mesh_parent, _transition_domain));
}

void
ClosedCoilSolver::SolveTransition()
{
  auto v_parent = std::make_shared<mfem::ParGridFunction>(_h1_fe_space_parent.get());
  *v_parent = 0.0;

  hephaestus::FESpaces fespaces;
  hephaestus::BCMap bc_maps;

  hephaestus::Coefficients coefs;
  coefs._scalars.Register("electrical_conductivity", _sigma);

  hephaestus::GridFunctions gridfunctions;
  gridfunctions.Register("ElectricField_parent", _source_electric_field);
  gridfunctions.Register("V_parent", v_parent);

  hephaestus::OpenCoilSolver opencoil("ElectricField_parent",
                                      "V_parent",
                                      "I",
                                      "electrical_conductivity",
                                      _transition_domain,
                                      _elec_attrs,
                                      true,
                                      "",
                                      _solver_options);

  opencoil.Init(gridfunctions, fespaces, bc_maps, coefs);
  opencoil.Apply(_final_lf.get());

  _mesh_coil->Transfer(*v_parent, *_v_coil);
}

void
ClosedCoilSolver::SolveCoil()
{
  // (σ∇Va,∇ψ) = -(σ∇Vt,∇ψ)
  // where Va is Vaux_coil_, the auxiliary continuous "potential"
  // ψ are the H1 test functions
  // Vt is the transition potential
  // The boundary terms are zero because ∇Va and ∇Vt are perpendicular
  // to the coil boundaries

  mfem::ParGridFunction vaux_coil(_h1_fe_space_coil.get());
  vaux_coil = 0.0;

  mfem::ParBilinearForm a_t(_h1_fe_space_coil.get());
  mfem::ParLinearForm b_coil(_h1_fe_space_coil.get());
  b_coil = 0.0;

  hephaestus::AttrToMarker(_transition_domain, _transition_markers, _mesh_coil->attributes.Max());
  a_t.AddDomainIntegrator(new mfem::DiffusionIntegrator(*_sigma), _transition_markers);
  a_t.Assemble();
  a_t.Finalize();
  a_t.AddMult(*_v_coil, b_coil, -1.0);

  mfem::ParBilinearForm a_coil(_h1_fe_space_coil.get());
  a_coil.AddDomainIntegrator(new mfem::DiffusionIntegrator(*_sigma));
  a_coil.Assemble();

  mfem::Array<int> ess_bdr_tdofs_coil;

  int ref_rank = -1;
  bool has_els = (bool)_mesh_coil->GetNE();

  // Gather all has_els info from the processes
  bool * all_has_els = (bool *)malloc(sizeof(bool) * mfem::Mpi::WorldSize());
  MPI_Allgather(&has_els, 1, MPI_CXX_BOOL, all_has_els, 1, MPI_CXX_BOOL, MPI_COMM_WORLD);

  for (int i = 0; i < mfem::Mpi::WorldSize(); ++i)
  {
    if (all_has_els[i] == true)
    {
      ref_rank = i;
      break;
    }
  }
  if (ref_rank == -1)
    mfem::mfem_error("Coil mesh has size zero!");

  free(all_has_els);

  if (mfem::Mpi::WorldRank() == ref_rank)
  {
    ess_bdr_tdofs_coil.SetSize(1);
    ess_bdr_tdofs_coil[0] = 0;
  }

  mfem::HypreParMatrix a0_coil;
  mfem::Vector x0_coil;
  mfem::Vector b0_coil;
  a_coil.FormLinearSystem(ess_bdr_tdofs_coil, vaux_coil, b_coil, a0_coil, x0_coil, b0_coil);
  hephaestus::DefaultH1PCGSolver a_coil_solver(_solver_options, a0_coil);
  a_coil_solver.Mult(b0_coil, x0_coil);
  a_coil.RecoverFEMSolution(x0_coil, b_coil, vaux_coil);

  // Now we form the final coil current
  mfem::ParDiscreteLinearOperator grad(_h1_fe_space_coil.get(),
                                       _electric_field_aux_coil->ParFESpace());
  grad.AddDomainInterpolator(new mfem::GradientInterpolator());
  grad.Assemble();
  grad.Mult(vaux_coil, *_electric_field_aux_coil);
  *_electric_field_aux_coil *= -1.0;

  if (_electric_field_transfer)
    _electric_field_t_parent = std::make_unique<mfem::ParGridFunction>(*_source_electric_field);

  *_source_electric_field = 0.0;
  _mesh_coil->Transfer(*_electric_field_aux_coil, *_source_electric_field);

  mfem::ParBilinearForm m1(_h_curl_fe_space_parent);
  hephaestus::AttrToMarker(_coil_domains, _coil_markers, _mesh_parent->attributes.Max());

  m1.AddDomainIntegrator(new mfem::VectorFEMassIntegrator(_sigma.get()), _coil_markers);
  m1.Assemble();
  m1.AddMult(*_source_electric_field, *_final_lf, 1.0);

  // We can't properly calculate the flux of Jaux on the parent mesh, so we
  // transfer it first to the transition mesh. This will be used in the
  // normalisation step
  auto electric_field_aux_t_fec =
      std::make_unique<mfem::ND_FECollection>(_order_hcurl, _mesh_t->Dimension());

  auto electric_field_aux_t_pfes =
      std::make_unique<mfem::ParFiniteElementSpace>(_mesh_t.get(), electric_field_aux_t_fec.get());

  auto electric_field_aux_t =
      std::make_unique<mfem::ParGridFunction>(electric_field_aux_t_pfes.get());
  *electric_field_aux_t = 0.0;

  _mesh_t->Transfer(*_source_electric_field, *electric_field_aux_t);

  // The total flux across the electrode face is Φ_t + Φ_aux
  // where Φ_t is the transition flux, already normalised to be -1
  double flux = -1.0 + calcFlux(electric_field_aux_t.get(), _elec_attrs.first, *_sigma);

  if (_electric_field_transfer)
  {
    *_source_electric_field += *_electric_field_t_parent;
    *_source_electric_field /= flux;
    *_electric_field_t_parent = *_source_electric_field;
  }

  *_final_lf /= flux;
}

void
ClosedCoilSolver::RestoreAttributes()
{
  // Domain attributes
  for (int e = 0; e < _mesh_parent->GetNE(); ++e)
  {
    _mesh_parent->SetAttribute(e, _old_dom_attrs[e]);
  }

  _mesh_parent->FinalizeTopology();
  _mesh_parent->Finalize();
  _mesh_parent->SetAttributes();
}

// Auxiliary methods

bool
ClosedCoilSolver::IsInDomain(const int el, const mfem::Array<int> & dom, const mfem::ParMesh * mesh)
{
  // This is for ghost elements
  if (el < 0)
    return false;

  bool verify = false;

  for (auto sd : dom)
  {
    if (mesh->GetAttribute(el) == sd)
      verify = true;
  }

  return verify;
}

bool
ClosedCoilSolver::IsInDomain(const int el, const int & sd, const mfem::ParMesh * mesh)
{
  // This is for ghost elements
  if (el < 0)
    return false;

  return mesh->GetAttribute(el) == sd;
}

mfem::Vector
ClosedCoilSolver::ElementCentre(int el, mfem::ParMesh * pm)
{
  mfem::Array<int> elem_vtx;
  mfem::Vector com(3);
  com = 0.0;

  pm->GetElementVertices(el, elem_vtx);

  for (auto vtx : elem_vtx)
  {
    for (int j = 0; j < 3; ++j)
      com[j] += pm->GetVertex(vtx)[j] / (double)elem_vtx.Size();
  }

  return com;
}

// 3D Plane constructor and methods

Plane3D::Plane3D()
{
  _u = std::make_unique<mfem::Vector>(3);
  *_u = 0.0;
}

void
Plane3D::Make3DPlane(const mfem::ParMesh * pm, const int face)
{
  MFEM_ASSERT(pm->Dimension() == 3, "Plane3D only works in 3-dimensional meshes!");

  mfem::Array<int> face_vtx;
  std::vector<mfem::Vector> v;
  pm->GetFaceVertices(face, face_vtx);

  // First we get the coordinates of 3 vertices on the face
  for (auto vtx : face_vtx)
  {
    mfem::Vector vtx_coords(3);
    for (int j = 0; j < 3; ++j)
      vtx_coords[j] = pm->GetVertex(vtx)[j];
    v.push_back(vtx_coords);
  }

  // Now we find the unit vector normal to the face
  v[0] -= v[1];
  v[1] -= v[2];
  v[0].cross3D(v[1], *_u);
  *_u /= _u->Norml2();

  // Finally, we find d:
  _d = *_u * v[2];
}

int
Plane3D::Side(const mfem::Vector v)
{
  double val = *_u * v - _d;

  if (val > 0)
    return 1;
  else if (val < 0)
    return -1;
  else
    return 0;
}

} // namespace hephaestus