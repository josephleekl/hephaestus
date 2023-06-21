#include "auxsolvers.hpp"
#include "transient_executioner.hpp"

#include "hephaestus.hpp"
#include "postprocessors.hpp"

#include <gtest/gtest.h>

extern const char *DATA_DIR;

class TestTeam7 : public testing::Test {
protected:
  static void source_current(const mfem::Vector &xv, double t,
                             mfem::Vector &J) {
    double x0(194e-3);  // Coil centre x coordinate
    double y0(100e-3);  // Coil centre y coordinate
    double a(50e-3);    // Coil thickness
    double I0(2742);    // Coil current in Ampere-turns
    double S(2.5e-3);   // Coil cross sectional area
    double freq(200.0); // Frequency in Hz

    double x = xv(0);
    double y = xv(1);

    // Current density magnitude
    double Jmag = (I0 / S) * sin(2 * M_PI * freq * t);

    // Calculate x component of current density unit vector
    if (abs(x - x0) < a) {
      J(0) = -(y - y0) / abs(y - y0);
    } else if (abs(y - y0) < a) {
      J(0) = 0.0;
    } else {
      J(0) = -(y - (y0 + a * ((y - y0) / abs(y - y0)))) /
             hypot(x - (x0 + a * ((x - x0) / abs(x - x0))),
                   y - (y0 + a * ((y - y0) / abs(y - y0))));
    }

    // Calculate y component of current density unit vector
    if (abs(y - y0) < a) {
      J(1) = (x - x0) / abs(x - x0);
    } else if (abs(x - x0) < a) {
      J(1) = 0.0;
    } else {
      J(1) = (x - (x0 + a * ((x - x0) / abs(x - x0)))) /
             hypot(x - (x0 + a * ((x - x0) / abs(x - x0))),
                   y - (y0 + a * ((y - y0) / abs(y - y0))));
    }

    // Calculate z component of current density unit vector
    J(2) = 0.0;

    // Scale by current density magnitude
    J *= Jmag;
  }

  hephaestus::InputParameters test_params() {
    hephaestus::Subdomain air("air", 1);
    air.property_map.Register("electrical_conductivity",
                              new mfem::ConstantCoefficient(1.0), true);
    hephaestus::Subdomain plate("plate", 2);
    plate.property_map.Register("electrical_conductivity",
                                new mfem::ConstantCoefficient(3.526e7), true);
    hephaestus::Subdomain coil1("coil1", 3);
    coil1.property_map.Register("electrical_conductivity",
                                new mfem::ConstantCoefficient(1.0), true);
    hephaestus::Subdomain coil2("coil2", 4);
    coil2.property_map.Register("electrical_conductivity",
                                new mfem::ConstantCoefficient(1.0), true);
    hephaestus::Subdomain coil3("coil3", 5);
    coil3.property_map.Register("electrical_conductivity",
                                new mfem::ConstantCoefficient(1.0), true);
    hephaestus::Subdomain coil4("coil4", 6);
    coil4.property_map.Register("electrical_conductivity",
                                new mfem::ConstantCoefficient(1.0), true);

    hephaestus::DomainProperties domain_properties(
        std::vector<hephaestus::Subdomain>(
            {air, plate, coil1, coil2, coil3, coil4}));

    domain_properties.scalar_property_map.Register(
        "magnetic_permeability", new mfem::ConstantCoefficient(M_PI * 4.0e-7),
        true);

    hephaestus::BCMap bc_map;

    mfem::Mesh mesh(
        (std::string(DATA_DIR) + std::string("./team7_small.g")).c_str(), 1, 1);

    std::map<std::string, mfem::DataCollection *> data_collections;
    data_collections["VisItDataCollection"] =
        new mfem::VisItDataCollection("Team7VisIt");
    data_collections["ParaViewDataCollection"] =
        new mfem::ParaViewDataCollection("Team7ParaView");
    hephaestus::Outputs outputs(data_collections);

    hephaestus::GridFunctions gridfunctions;
    hephaestus::AuxSolvers postprocessors;
    hephaestus::AuxSolvers preprocessors;

    hephaestus::Sources sources;
    mfem::VectorFunctionCoefficient *JSrcCoef =
        new mfem::VectorFunctionCoefficient(3, source_current);
    mfem::Array<mfem::VectorCoefficient *> sourcecoefs(4);
    sourcecoefs[0] = JSrcCoef;
    sourcecoefs[1] = JSrcCoef;
    sourcecoefs[2] = JSrcCoef;
    sourcecoefs[3] = JSrcCoef;
    mfem::Array<int> coilsegments(4);
    coilsegments[0] = 3;
    coilsegments[1] = 4;
    coilsegments[2] = 5;
    coilsegments[3] = 6;
    mfem::PWVectorCoefficient *JSrcRestricted =
        new mfem::PWVectorCoefficient(3, coilsegments, sourcecoefs);
    domain_properties.vector_property_map.Register("source", JSrcRestricted,
                                                   true);

    hephaestus::InputParameters div_free_source_params;
    div_free_source_params.SetParam("SourceName", std::string("source"));
    div_free_source_params.SetParam("HCurlFESpaceName",
                                    std::string("_HCurlFESpace"));
    div_free_source_params.SetParam("H1FESpaceName", std::string("H1"));
    hephaestus::InputParameters current_solver_options;
    current_solver_options.SetParam("Tolerance", float(1.0e-12));
    current_solver_options.SetParam("MaxIter", (unsigned int)200);
    current_solver_options.SetParam("PrintLevel", 0);
    div_free_source_params.SetParam("SolverOptions", current_solver_options);
    sources.Register(
        "source", new hephaestus::DivFreeSource(div_free_source_params), true);

    hephaestus::InputParameters solver_options;
    solver_options.SetParam("Tolerance", float(1.0e-16));
    solver_options.SetParam("MaxIter", (unsigned int)1000);
    solver_options.SetParam("PrintLevel", 0);

    hephaestus::InputParameters params;
    params.SetParam("Mesh", mfem::ParMesh(MPI_COMM_WORLD, mesh));
    params.SetParam("BoundaryConditions", bc_map);
    params.SetParam("DomainProperties", domain_properties);
    params.SetParam("GridFunctions", gridfunctions);
    params.SetParam("PreProcessors", preprocessors);
    params.SetParam("PostProcessors", postprocessors);
    params.SetParam("Outputs", outputs);
    params.SetParam("Sources", sources);
    params.SetParam("SolverOptions", solver_options);
    std::cout << "Created params ";
    return params;
  }
};

TEST_F(TestTeam7, CheckRun) {
  hephaestus::InputParameters params(test_params());

  hephaestus::TimeDomainProblemBuilder *problem_builder =
      new hephaestus::AFormulation();
  hephaestus::BCMap bc_map(
      params.GetParam<hephaestus::BCMap>("BoundaryConditions"));
  hephaestus::DomainProperties domain_properties(
      params.GetParam<hephaestus::DomainProperties>("DomainProperties"));
  hephaestus::AuxSolvers preprocessors(
      params.GetParam<hephaestus::AuxSolvers>("PreProcessors"));
  hephaestus::AuxSolvers postprocessors(
      params.GetParam<hephaestus::AuxSolvers>("PostProcessors"));
  hephaestus::Sources sources(params.GetParam<hephaestus::Sources>("Sources"));
  hephaestus::Outputs outputs(params.GetParam<hephaestus::Outputs>("Outputs"));
  hephaestus::InputParameters solver_options(
      params.GetOptionalParam<hephaestus::InputParameters>(
          "SolverOptions", hephaestus::InputParameters()));

  std::shared_ptr<mfem::ParMesh> pmesh =
      std::make_shared<mfem::ParMesh>(params.GetParam<mfem::ParMesh>("Mesh"));
  problem_builder->SetMesh(pmesh);
  problem_builder->AddFESpace(std::string("H1"), std::string("H1_3D_P2"));
  problem_builder->AddFESpace(std::string("HDiv"), std::string("RT_3D_P0"));
  problem_builder->AddGridFunction(std::string("magnetic_flux_density"),
                                   std::string("HDiv"));
  problem_builder->SetBoundaryConditions(bc_map);
  problem_builder->SetAuxSolvers(preprocessors);
  problem_builder->SetCoefficients(domain_properties);
  problem_builder->SetPostprocessors(postprocessors);
  problem_builder->SetSources(sources);
  problem_builder->SetOutputs(outputs);
  problem_builder->SetSolverOptions(solver_options);

  hephaestus::ProblemBuildSequencer sequencer(problem_builder);
  sequencer.ConstructEquationSystemProblem();
  std::unique_ptr<hephaestus::TimeDomainProblem> problem =
      problem_builder->ReturnProblem();
  hephaestus::InputParameters exec_params;
  exec_params.SetParam("TimeStep", float(0.001));
  exec_params.SetParam("StartTime", float(0.00));
  exec_params.SetParam("EndTime", float(0.002));
  exec_params.SetParam("VisualisationSteps", int(1));
  exec_params.SetParam("UseGLVis", false);
  exec_params.SetParam("Problem", problem.get());
  hephaestus::TransientExecutioner *executioner =
      new hephaestus::TransientExecutioner(exec_params);

  std::cout << "Created exec ";
  executioner->Init();
  executioner->Execute();
}
