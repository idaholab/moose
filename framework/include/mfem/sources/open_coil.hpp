#pragma once
#include "helmholtz_projector.hpp"
#include "scalar_potential_source.hpp"
#include "flux_monitor_aux.hpp"
#include "scaled_vector_gridfunction_aux.hpp"
#include "source_base.hpp"

namespace hephaestus
{

double highV(const mfem::Vector & x, double t);
double lowV(const mfem::Vector & x, double t);

void inheritBdrAttributes(const mfem::ParMesh * parent_mesh, mfem::ParSubMesh * child_mesh);

// Applies the HelmholtzProjector onto the J GridFunction to clean it of any
// divergences. This is for the simplest case with no BCs
void cleanDivergence(std::shared_ptr<mfem::ParGridFunction> Vec_GF,
                     hephaestus::InputParameters solve_pars);

// The more complicated case where BCs are needed
void cleanDivergence(const hephaestus::GridFunctions & gfs,
                     const hephaestus::BCMap & bcs,
                     const std::string vec_gf_name,
                     const std::string scalar_gf_name,
                     hephaestus::InputParameters solve_pars);

class OpenCoilSolver : public hephaestus::Source
{

public:
  OpenCoilSolver(std::string source_efield_gf_name,
                 std::string electric_potential_name,
                 std::string i_coef_name,
                 std::string cond_coef_name,
                 mfem::Array<int> coil_dom,
                 const std::pair<int, int> electrodes,
                 bool electric_field_transfer = true,
                 std::string source_jfield_gf_name = "",
                 hephaestus::InputParameters solver_options =
                     hephaestus::InputParameters({{"Tolerance", float(1.0e-20)},
                                                  {"AbsTolerance", float(1.0e-20)},
                                                  {"MaxIter", (unsigned int)1000},
                                                  {"PrintLevel", GetGlobalPrintLevel()}}));

  ~OpenCoilSolver() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            hephaestus::Coefficients & coefficients) override;
  void Apply(mfem::ParLinearForm * lf) override;
  void SubtractSource(mfem::ParGridFunction * gf) override{};

  // Initialises the child submesh.
  void InitChildMesh();

  // Creates the relevant FE Collections and Spaces for the child submesh.
  void MakeFESpaces();

  // Creates the relevant GridFunctions for the child submesh.
  void MakeGridFunctions();

  // Sets up the boundary conditions to be used in the ScalarPotentialSource.
  // calculation.
  void SetBCs();

  // Solves for the divergence-free Hodge dual of the electric current based on
  // Dirichlet BCs.
  void SPSCurrent();

  // Creates a mass matrix with basis functions that will be used in the Apply()
  // method
  void BuildM1();

  // Sets the boundary attribute for the face to be used as reference in flux
  // calculation
  void SetRefFace(const int face);

private:
  // Parameters
  std::pair<int, int> _elec_attrs;
  mfem::Array<int> _coil_domains;
  mfem::Array<int> _coil_markers;
  hephaestus::InputParameters _solver_options;

  int _order_h1;
  int _order_hcurl;
  int _order_hdiv;
  int _ref_face;
  bool _electric_field_transfer;

  std::shared_ptr<mfem::Coefficient> _sigma{nullptr};
  std::shared_ptr<mfem::Coefficient> _itotal{nullptr};

  // Names
  std::string _grad_phi_name;
  std::string _phi_gf_name;
  std::string _i_coef_name;
  std::string _cond_coef_name;
  std::string _source_efield_gf_name;
  std::string _source_jfield_gf_name;

  // Parent mesh, FE space, and current
  mfem::ParMesh * _mesh_parent{nullptr};

  mfem::ParGridFunction * _grad_phi_parent{nullptr};
  std::unique_ptr<mfem::ParGridFunction> _grad_phi_t_parent{nullptr};

  std::shared_ptr<mfem::ParGridFunction> _phi_parent{nullptr};
  std::unique_ptr<mfem::ParGridFunction> _phi_t_parent{nullptr};

  std::shared_ptr<mfem::ParGridFunction> _j_parent{nullptr};
  std::unique_ptr<mfem::ParGridFunction> _j_t_parent{nullptr};

  std::shared_ptr<mfem::ParGridFunction> _source_electric_field{nullptr};
  std::shared_ptr<mfem::ParGridFunction> _source_current_density{nullptr};

  // Child mesh and FE spaces
  std::unique_ptr<mfem::ParSubMesh> _mesh_child{nullptr};

  std::shared_ptr<mfem::ParFiniteElementSpace> _h1_fe_space_child{nullptr};
  std::unique_ptr<mfem::H1_FECollection> _h1_fe_space_fec_child{nullptr};

  std::shared_ptr<mfem::ParFiniteElementSpace> _h_curl_fe_space_child{nullptr};
  std::unique_ptr<mfem::ND_FECollection> _h_curl_fe_space_fec_child{nullptr};

  std::shared_ptr<mfem::ParFiniteElementSpace> _h_div_fe_space_child{nullptr};
  std::unique_ptr<mfem::RT_FECollection> _h_div_fe_space_fec_child{nullptr};

  // Child GridFunctions
  std::shared_ptr<mfem::ParGridFunction> _grad_phi_child{nullptr};
  std::shared_ptr<mfem::ParGridFunction> _phi_child{nullptr};
  std::shared_ptr<mfem::ParGridFunction> _j_child{nullptr};

  // Child boundary condition objects
  std::shared_ptr<mfem::FunctionCoefficient> _high_src{nullptr};
  std::shared_ptr<mfem::FunctionCoefficient> _low_src{nullptr};

  mfem::Array<int> _high_terminal;
  mfem::Array<int> _low_terminal;

  // Mass Matrix
  std::unique_ptr<mfem::ParBilinearForm> _m1{nullptr};

  // BC Map
  hephaestus::BCMap _bc_maps;

  // Final LinearForm
  std::unique_ptr<mfem::ParLinearForm> _final_lf{nullptr};
};

} // namespace hephaestus