#pragma once
#include "../common/pfem_extras.hpp"
#include "formulation.hpp"
#include "inputs.hpp"
#include "sources.hpp"

namespace hephaestus
{

class AVFormulation : public TimeDomainEMFormulation
{
public:
  AVFormulation(std::string alpha_coef_name,
                std::string inv_alpha_coef_name,
                std::string beta_coef_name,
                std::string vector_potential_name,
                std::string scalar_potential_name);

  ~AVFormulation() override = default;

  void ConstructOperator() override;

  void RegisterGridFunctions() override;

  void RegisterCoefficients() override;

protected:
  const std::string _alpha_coef_name;
  const std::string _inv_alpha_coef_name;
  const std::string _beta_coef_name;
  const std::string _vector_potential_name;
  const std::string _scalar_potential_name;
};

class AVEquationSystem : public TimeDependentEquationSystem
{
public:
  AVEquationSystem(const hephaestus::InputParameters & params);

  ~AVEquationSystem() override = default;

  void Init(hephaestus::GridFunctions & gridfunctions,
            const hephaestus::FESpaces & fespaces,
            hephaestus::BCMap & bc_map,
            hephaestus::Coefficients & coefficients) override;
  void AddKernels() override;

  std::string _a_name, _v_name, _coupled_variable_name, _alpha_coef_name, _beta_coef_name,
      _dtalpha_coef_name, _neg_beta_coef_name;
  mfem::ConstantCoefficient _neg_coef;
};

} // namespace hephaestus
