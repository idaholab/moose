#include "open_coil.hpp"
#include "utils.hpp"

#include <utility>

namespace hephaestus
{

double
highV(const mfem::Vector & x, double t)
{
  return 0.5;
}
double
lowV(const mfem::Vector & x, double t)
{
  return -0.5;
}

OpenCoilSolver::OpenCoilSolver(std::string source_efield_gf_name,
                               std::string phi_gf_name,
                               std::string i_coef_name,
                               std::string cond_coef_name,
                               mfem::Array<int> coil_dom,
                               const std::pair<int, int> electrodes,
                               bool electric_field_transfer,
                               std::string source_jfield_gf_name,
                               hephaestus::InputParameters solver_options)
  : _source_efield_gf_name(std::move(source_efield_gf_name)),
    _source_jfield_gf_name(std::move(source_jfield_gf_name)),
    _phi_gf_name(std::move(phi_gf_name)),
    _i_coef_name(std::move(i_coef_name)),
    _cond_coef_name(std::move(cond_coef_name)),
    _electric_field_transfer(std::move(electric_field_transfer)),
    _solver_options(std::move(solver_options)),
    _coil_domains(std::move(coil_dom)),
    _elec_attrs(electrodes),
    _high_src(std::make_shared<mfem::FunctionCoefficient>(highV)),
    _low_src(std::make_shared<mfem::FunctionCoefficient>(lowV)),
    _high_terminal(1),
    _low_terminal(1),
    _ref_face(_elec_attrs.first)
{
}

void
OpenCoilSolver::Init(hephaestus::GridFunctions & gridfunctions,
                     const hephaestus::FESpaces & fespaces,
                     hephaestus::BCMap & bc_map,
                     hephaestus::Coefficients & coefficients)
{
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
    logger.warn("Source electric field undefined. The GridFunction associated with it will be set "
                "to zero.");

    _sigma = std::make_shared<mfem::ConstantCoefficient>(1.0);

    _electric_field_transfer = false;
  }
  else
  {
    _sigma = coefficients._scalars.GetShared(_cond_coef_name);
  }

  _source_electric_field = gridfunctions.GetShared(_source_efield_gf_name);

  if (_source_electric_field->ParFESpace()->FEColl()->GetContType() !=
      mfem::FiniteElementCollection::TANGENTIAL)
  {
    mfem::mfem_error("Electric field GridFunction must be of HCurl type.");
  }

  if (!_source_jfield_gf_name.empty())
  {
    _source_current_density = gridfunctions.GetShared(_source_jfield_gf_name);
    if (_source_current_density == nullptr)
    {
      const std::string error_message = _source_jfield_gf_name + " not found in gridfunctions when "
                                                                 "creating OpenCoilSolver\n";
      mfem::mfem_error(error_message.c_str());
    }
    else if (_source_current_density->ParFESpace()->FEColl()->GetContType() !=
             mfem::FiniteElementCollection::NORMAL)
    {
      mfem::mfem_error("Current density GridFunction must be of HDiv type.");
    }
    _order_hdiv = _source_current_density->ParFESpace()->FEColl()->GetOrder();
  }

  _order_hcurl = _source_electric_field->ParFESpace()->FEColl()->GetOrder();

  _phi_parent = gridfunctions.GetShared(_phi_gf_name);
  if (_phi_parent->ParFESpace()->FEColl()->GetContType() !=
      mfem::FiniteElementCollection::CONTINUOUS)
  {
    mfem::mfem_error("V GridFunction must be of H1 type.");
  }
  else
  {
    _order_h1 = _phi_parent->ParFESpace()->FEColl()->GetOrder();
    _phi_t_parent = std::make_unique<mfem::ParGridFunction>(*_phi_parent);
  }

  _mesh_parent = _source_electric_field->ParFESpace()->GetParMesh();

  InitChildMesh();
  MakeFESpaces();
  MakeGridFunctions();
  SetBCs();
  SPSCurrent();
}

void
OpenCoilSolver::Apply(mfem::ParLinearForm * lf)
{

  // The transformation and integration points themselves are not relevant, it's
  // just so we can call Eval
  mfem::ElementTransformation * tr = _mesh_parent->GetElementTransformation(0);
  const mfem::IntegrationPoint & ip =
      mfem::IntRules.Get(_source_electric_field->ParFESpace()->GetFE(0)->GetGeomType(), 1)
          .IntPoint(0);

  double i = _itotal->Eval(*tr, ip);

  if (_electric_field_transfer)
  {
    *_source_electric_field = 0.0;
    _source_electric_field->Add(-i, *_grad_phi_t_parent);
  }

  if (_phi_parent != nullptr)
  {
    *_phi_parent = 0.0;
    _phi_parent->Add(i, *_phi_t_parent);
  }

  if (_source_current_density)
  {
    *_source_current_density = 0.0;
    _source_current_density->Add(i, *_j_t_parent);
  }

  lf->Add(i, *_final_lf);
}

void
OpenCoilSolver::InitChildMesh()
{
  if (_mesh_child == nullptr)
    _mesh_child = std::make_unique<mfem::ParSubMesh>(
        mfem::ParSubMesh::CreateFromDomain(*_mesh_parent, _coil_domains));
}

void
OpenCoilSolver::MakeFESpaces()
{
  if (_h1_fe_space_child == nullptr)
  {
    _h1_fe_space_fec_child =
        std::make_unique<mfem::H1_FECollection>(_order_h1, _mesh_child->Dimension());
    _h1_fe_space_child = std::make_shared<mfem::ParFiniteElementSpace>(
        _mesh_child.get(), _h1_fe_space_fec_child.get());
  }

  if (_h_curl_fe_space_child == nullptr)
  {
    _h_curl_fe_space_fec_child =
        std::make_unique<mfem::ND_FECollection>(_order_hcurl, _mesh_child->Dimension());
    _h_curl_fe_space_child = std::make_shared<mfem::ParFiniteElementSpace>(
        _mesh_child.get(), _h_curl_fe_space_fec_child.get());
  }

  if (_source_current_density && _h_div_fe_space_child == nullptr)
  {
    _h_div_fe_space_fec_child =
        std::make_unique<mfem::RT_FECollection>(_order_hdiv - 1, _mesh_child->Dimension());
    _h_div_fe_space_child = std::make_shared<mfem::ParFiniteElementSpace>(
        _mesh_child.get(), _h_div_fe_space_fec_child.get());
  }
}

void
OpenCoilSolver::MakeGridFunctions()
{

  if (_phi_child == nullptr)
    _phi_child = std::make_shared<mfem::ParGridFunction>(_h1_fe_space_child.get());

  if (_grad_phi_child == nullptr)
    _grad_phi_child = std::make_shared<mfem::ParGridFunction>(_h_curl_fe_space_child.get());

  if (_grad_phi_t_parent == nullptr)
    _grad_phi_t_parent = std::make_unique<mfem::ParGridFunction>(*_source_electric_field);

  if (_source_current_density)
  {
    if (_j_child == nullptr)
      _j_child = std::make_unique<mfem::ParGridFunction>(_h_div_fe_space_child.get());

    if (_j_t_parent == nullptr)
      _j_t_parent = std::make_unique<mfem::ParGridFunction>(*_source_current_density);
    *_j_child = 0.0;
    *_j_t_parent = 0.0;
  }

  *_phi_child = 0.0;
  *_grad_phi_child = 0.0;
  *_grad_phi_t_parent = 0.0;
}

void
OpenCoilSolver::SetBCs()
{

  _high_terminal[0] = _elec_attrs.first;
  _low_terminal[0] = _elec_attrs.second;
}

void
OpenCoilSolver::SPSCurrent()
{
  _bc_maps.Register("high_potential",
                    std::make_shared<hephaestus::ScalarDirichletBC>(
                        std::string("V"), _high_terminal, _high_src.get()));

  _bc_maps.Register("low_potential",
                    std::make_shared<hephaestus::ScalarDirichletBC>(
                        std::string("V"), _low_terminal, _low_src.get()));

  hephaestus::FESpaces fespaces;
  fespaces.Register("HCurl", _h_curl_fe_space_child);
  fespaces.Register("H1", _h1_fe_space_child);

  hephaestus::GridFunctions gridfunctions;
  gridfunctions.Register("GradPhi", _grad_phi_child);
  gridfunctions.Register("V", _phi_child);

  hephaestus::Coefficients coefs;
  coefs._scalars.Register("electric_conductivity", _sigma);

  hephaestus::ScalarPotentialSource sps(
      "GradPhi", "V", "HCurl", "H1", "electric_conductivity", 1, _solver_options);
  sps.Init(gridfunctions, fespaces, _bc_maps, coefs);

  mfem::ParLinearForm dummy(_h_curl_fe_space_child.get());
  sps.Apply(&dummy);

  // Normalise the current through the wedges and use them as a reference
  double flux = calcFlux(_grad_phi_child.get(), _ref_face, *_sigma);
  *_grad_phi_child /= abs(flux);
  if (_phi_child)
    *_phi_child /= abs(flux);

  _mesh_child->Transfer(*_grad_phi_child, *_grad_phi_t_parent);
  if (_phi_parent)
    _mesh_child->Transfer(*_phi_child, *_phi_t_parent);

  if (_source_current_density)
  {
    hephaestus::GridFunctions aux_gf;
    aux_gf.Register("grad_phi_child", _grad_phi_child);
    aux_gf.Register("source_current_density", _j_child);

    hephaestus::Coefficients aux_coef;
    aux_coef._scalars.Register("electrical_conductivity", _sigma);

    hephaestus::ScaledVectorGridFunctionAux current_density_auxsolver(
        "grad_phi_child", "source_current_density", "electrical_conductivity", -1.0);
    current_density_auxsolver.Init(aux_gf, aux_coef);
    current_density_auxsolver.Solve();

    _mesh_child->Transfer(*_j_child, *_j_t_parent);
  }

  BuildM1();

  _final_lf = std::make_unique<mfem::ParLinearForm>(_grad_phi_t_parent->ParFESpace());
  *_final_lf = 0.0;
  _m1->AddMult(*_grad_phi_t_parent, *_final_lf, -1.0);
}

void
OpenCoilSolver::BuildM1()
{
  if (_m1 == nullptr)
  {
    _m1 = std::make_unique<mfem::ParBilinearForm>(_source_electric_field->ParFESpace());
    hephaestus::AttrToMarker(_coil_domains, _coil_markers, _mesh_parent->attributes.Max());
    _m1->AddDomainIntegrator(new mfem::VectorFEMassIntegrator(_sigma.get()), _coil_markers);
    _m1->Assemble();
    _m1->Finalize();
  }
}

void
OpenCoilSolver::SetRefFace(const int face)
{
  _ref_face = face;
}

} // namespace hephaestus