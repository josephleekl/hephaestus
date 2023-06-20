#pragma once
#include "../common/pfem_extras.hpp"
#include "formulation.hpp"
#include "inputs.hpp"
#include "sources.hpp"

namespace hephaestus {

class AVFormulation : public TimeDomainFormulation {
  std::string vector_potential_name, scalar_potential_name, alpha_coef_name,
      beta_coef_name;

public:
  AVFormulation();

  virtual void ConstructEquationSystem() override;

  virtual void ConstructOperator() override;

  virtual void RegisterGridFunctions() override;

  virtual void RegisterCoefficients() override;
};

class AVOperator : public TimeDomainEquationSystemOperator {
public:
  AVOperator(mfem::ParMesh &pmesh,
             mfem::NamedFieldsMap<mfem::ParFiniteElementSpace> &fespaces,
             mfem::NamedFieldsMap<mfem::ParGridFunction> &variables,
             hephaestus::BCMap &bc_map,
             hephaestus::DomainProperties &domain_properties,
             hephaestus::Sources &sources,
             hephaestus::InputParameters &solver_options);

  ~AVOperator(){};

  void ImplicitSolve(const double dt, const mfem::Vector &X,
                     mfem::Vector &dX_dt) override;
};
} // namespace hephaestus
