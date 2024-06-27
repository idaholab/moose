#pragma once
#include "mesh_extras.hpp"
#include "named_fields_map.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_set>

namespace hephaestus
{

double prodFunc(double a, double b);
double fracFunc(double a, double b);

class Subdomain
{
public:
  Subdomain(std::string name_, int id_);

  std::string _name;
  int _id;
  hephaestus::NamedFieldsMap<mfem::Coefficient> _scalar_coefficients;
  hephaestus::NamedFieldsMap<mfem::VectorCoefficient> _vector_coefficients;
};

// Coefficients - stores all scalar and vector coefficients
//--SetTime
//--scalars
//--vectors

// Stores all coefficients defined over
class Coefficients
{
  double _t; // Time at which time-dependent coefficients are evaluated
public:
  Coefficients();
  ~Coefficients() = default;

  Coefficients(std::vector<Subdomain> subdomains_);
  void SetTime(double t);
  void AddGlobalCoefficientsFromSubdomains();
  void RegisterDefaultCoefficients();

  hephaestus::NamedFieldsMap<mfem::Coefficient> _scalars;
  hephaestus::NamedFieldsMap<mfem::VectorCoefficient> _vectors;
  std::vector<Subdomain> _subdomains;
};

} // namespace hephaestus
