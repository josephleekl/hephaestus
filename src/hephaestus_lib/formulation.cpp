#include "formulation.hpp"

namespace hephaestus {

void TransientFormulation::Init(mfem::Vector &X) {
  // Define material property coefficients
  SetMaterialCoefficients(_domain_properties);
  SetEquationSystem();
  _sources.Init(_variables, _fespaces, _bc_map, _domain_properties);

  for (unsigned int ind = 0; ind < local_test_vars.size(); ++ind) {
    local_test_vars.at(ind)->MakeRef(local_test_vars.at(ind)->ParFESpace(),
                                     const_cast<mfem::Vector &>(X),
                                     true_offsets[ind]);
    *(local_test_vars.at(ind)) = 0.0;
    *(local_trial_vars.at(ind)) = 0.0;
  }

  _equation_system->Init(_variables, _fespaces, _bc_map, _domain_properties);
  _equation_system->buildEquationSystem(_bc_map, _sources);
};

void TransientFormulation::RegisterVariables() {
  RegisterMissingVariables();
  local_test_vars = populateVectorFromNamedFieldsMap<mfem::ParGridFunction>(
      _variables, state_var_names);
  local_trial_vars = registerTimeDerivatives(state_var_names, _variables);

  // Set operator size and block structure
  block_trueOffsets.SetSize(local_test_vars.size() + 1);
  block_trueOffsets[0] = 0;
  for (unsigned int ind = 0; ind < local_test_vars.size(); ++ind) {
    block_trueOffsets[ind + 1] =
        local_test_vars.at(ind)->ParFESpace()->TrueVSize();
  }
  block_trueOffsets.PartialSum();

  true_offsets.SetSize(local_test_vars.size() + 1);
  true_offsets[0] = 0;
  for (unsigned int ind = 0; ind < local_test_vars.size(); ++ind) {
    true_offsets[ind + 1] = local_test_vars.at(ind)->ParFESpace()->GetVSize();
  }
  true_offsets.PartialSum();

  this->height = true_offsets[local_test_vars.size()];
  this->width = true_offsets[local_test_vars.size()];
  trueX.Update(block_trueOffsets);
  trueRhs.Update(block_trueOffsets);

  // Populate vector of active auxiliary variables
  active_aux_var_names.resize(0);
  for (auto &aux_var_name : aux_var_names) {
    if (_variables.Has(aux_var_name)) {
      active_aux_var_names.push_back(aux_var_name);
    }
  }
};

std::string TransientFormulation::GetTimeDerivativeName(std::string name) {
  return std::string("d") + name + std::string("_dt");
}

std::vector<std::string> TransientFormulation::GetTimeDerivativeNames(
    std::vector<std::string> gridfunction_names) {
  std::vector<std::string> time_derivative_names;
  for (auto &gridfunction_name : gridfunction_names) {
    time_derivative_names.push_back(GetTimeDerivativeName(gridfunction_name));
  }
  return time_derivative_names;
}

std::vector<mfem::ParGridFunction *>
TransientFormulation::registerTimeDerivatives(
    std::vector<std::string> gridfunction_names,
    mfem::NamedFieldsMap<mfem::ParGridFunction> &gridfunctions) {
  std::vector<mfem::ParGridFunction *> time_derivatives;

  for (auto &gridfunction_name : gridfunction_names) {
    gridfunctions.Register(
        GetTimeDerivativeName(gridfunction_name),
        new mfem::ParGridFunction(
            gridfunctions.Get(gridfunction_name)->ParFESpace()),
        true);
    time_derivatives.push_back(
        gridfunctions.Get(GetTimeDerivativeName(gridfunction_name)));
  }
  return time_derivatives;
}
} // namespace hephaestus