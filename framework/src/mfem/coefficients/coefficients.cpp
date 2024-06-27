#include "coefficients.hpp"

#include <utility>

namespace hephaestus
{

double
prodFunc(double a, double b)
{
  return a * b;
}
double
fracFunc(double a, double b)
{
  return a / b;
}

Subdomain::Subdomain(std::string name_, int id_) : _name(std::move(name_)), _id(id_) {}

Coefficients::Coefficients() { RegisterDefaultCoefficients(); }

Coefficients::Coefficients(std::vector<Subdomain> subdomains_) : _subdomains(std::move(subdomains_))
{
  AddGlobalCoefficientsFromSubdomains();
  RegisterDefaultCoefficients();
}

void
Coefficients::RegisterDefaultCoefficients()
{
  _scalars.Register("_one", std::make_shared<mfem::ConstantCoefficient>(1.0));
}

void
Coefficients::SetTime(double time)
{
  for (auto const & [name, coeff_] : _scalars)
  {
    coeff_->SetTime(time);
  }
  for (auto const & [name, vec_coeff_] : _vectors)
  {
    vec_coeff_->SetTime(time);
  }
  _t = time;
}

// merge subdomains?
void
Coefficients::AddGlobalCoefficientsFromSubdomains()
{

  // iterate over subdomains
  // check IDs span domain
  // accumulate list of property_name in unordered map

  // for each property_name
  // iterate over subdomains
  // accumulate coefs
  mfem::Array<int> subdomain_ids;
  std::unordered_set<std::string> scalar_property_names;
  std::unordered_set<std::string> vector_property_names;

  for (auto & subdomain : _subdomains)
  {
    subdomain_ids.Append(subdomain._id);
    // accumulate property names on subdomains, ignoring duplicates
    for (auto const & [name, coeff_] : subdomain._scalar_coefficients)
    {
      scalar_property_names.insert(name);
    }

    for (auto const & [name, coeff_] : subdomain._vector_coefficients)
    {
      vector_property_names.insert(name);
    }
  }
  // check if IDs span
  // iterate over properties stored on subdomains, and create global
  // coefficients
  for (auto & scalar_property_name : scalar_property_names)
  {
    mfem::Array<mfem::Coefficient *> subdomain_coefs;
    for (auto & subdomain : _subdomains)
    {
      subdomain_coefs.Append(subdomain._scalar_coefficients.Get(scalar_property_name));
    }
    if (!_scalars.Has(scalar_property_name))
    {
      _scalars.Register(scalar_property_name,
                        std::make_shared<mfem::PWCoefficient>(subdomain_ids, subdomain_coefs));
    }
  }

  for (auto & vector_property_name : vector_property_names)
  {
    mfem::Array<mfem::VectorCoefficient *> subdomain_coefs;
    for (auto & subdomain : _subdomains)
    {
      subdomain_coefs.Append(subdomain._vector_coefficients.Get(vector_property_name));
    }
    if (!_vectors.Has(vector_property_name))
    {
      _vectors.Register(
          vector_property_name,
          std::make_shared<mfem::PWVectorCoefficient>(3, subdomain_ids, subdomain_coefs));
    }
  }
}
} // namespace hephaestus
