#include "header.h"

//Rotational functions
double r_f(const Vector &x);
void rot_f(const Vector &x, DenseMatrix &f);
void zero_f(const Vector &x, Vector &f);

//Fusion temperature dependent of salinity
double T_fun(const double &salinity);

//Variation on solid heat capacity
double delta_c_s_fun(const double &temperature, const double &salinity);

void Artic_sea::assemble_system(){
    //Initialize the system
    t = 0;
    dt = config.dt_init;
    last = false;
    vis_steps = config.vis_steps_max;
    vis_impressions = 0;

    //Define solution x
    theta = new ParGridFunction(fespace);
    phi = new ParGridFunction(fespace);
    phase = new ParGridFunction(fespace);

    oper_T = new Conduction_Operator(config, *fespace, dim, pmesh->bdr_attributes.Max(), block_true_offsets, X);

    //Recover initial conditions
    theta->Distribute(&(X.GetBlock(0)));
    phi->Distribute(&(X.GetBlock(1)));

    //Calculate phases
    for (int ii = 0; ii < phase->Size(); ii++){
        double T_f = config.T_f + T_fun((*phi)(ii));
        (*phase)(ii) = 0.5*(1 + tanh(5*config.invDeltaT*((*theta)(ii) - T_f)));
    }

    //Set the ODE solver type
    switch (config.ode_solver_type){
        // MFEM explicit methods
        case 1: ode_solver = new ForwardEulerSolver; break;
        case 2: ode_solver = new RK2Solver(0.5); break;
        case 3: ode_solver = new RK3SSPSolver; break;
        case 4: ode_solver = new RK4Solver; break;
        // MFEM implicit L-stable methods
        case 5: ode_solver = new BackwardEulerSolver; break;
        case 6: ode_solver = new SDIRK23Solver(2); break;
        case 7: ode_solver = new SDIRK33Solver; break;
        // CVODE
        case 8: cvode = new CVODESolver(MPI_COMM_WORLD, CV_ADAMS); break;
        case 9: cvode = new CVODESolver(MPI_COMM_WORLD, CV_BDF); break;
        // ARKODE
        case 10: arkode = new ARKStepSolver(MPI_COMM_WORLD, ARKStepSolver::EXPLICIT); break;
        case 11: arkode = new ARKStepSolver(MPI_COMM_WORLD, ARKStepSolver::EXPLICIT); break;
        case 12: arkode = new ARKStepSolver(MPI_COMM_WORLD, ARKStepSolver::IMPLICIT); break;
        default:
                 cout << "Unknown ODE solver type: " << config.ode_solver_type << "\n"
                      << "Setting ODE to BackwardEulerSolver.\n";
                 config.ode_solver_type = 1;
                 ode_solver = new BackwardEulerSolver; break;
    }

    // Initialize ODE solver
    if (config.ode_solver_type < 8)
        ode_solver->Init(*oper_T);
    else if (cvode){
        cvode->Init(*oper_T);
        cvode->SetSStolerances(config.reltol_sundials, config.abstol_sundials);
        cvode->SetMaxStep(dt);
        cvode->SetStepMode(CV_ONE_STEP);
        ode_solver = cvode;
    }
    else if (arkode){
        arkode->Init(*oper_T);
        arkode->SetSStolerances(config.reltol_sundials, config.abstol_sundials);
        arkode->SetMaxStep(dt);
        arkode->SetStepMode(ARK_ONE_STEP);
        if (config.ode_solver_type == 11) arkode->SetERKTableNum(FEHLBERG_13_7_8);
        ode_solver = arkode;
    }

    //Open the paraview output and print initial state
    string folder = "results/graph"; 
    paraview_out = new ParaViewDataCollection(folder, pmesh);
    paraview_out->SetDataFormat(VTKFormat::BINARY);
    paraview_out->SetLevelsOfDetail(config.order);
    paraview_out->RegisterField("Temperature", theta);
    paraview_out->RegisterField("Salinity", phi);
    paraview_out->RegisterField("Phase", phase);
    paraview_out->SetCycle(vis_impressions);
    paraview_out->SetTime(t);
    paraview_out->Save();

    //Start program check
    if (config.master)
        cout << left << setw(12)
             << "--------------------------------------------------\n"
             << left << setw(12)
             << "Step" << setw(12)
             << "Dt" << setw(12)
             << "Time" << setw(12)
             << "Progress"
             << left << setw(12)
             << "\n--------------------------------------------------\n";

}

double r_f(const Vector &x){
    return x(0);
}

void rot_f(const Vector &x, DenseMatrix &f){
    f(0,0) = 0.; f(0,1) = -1.;
    f(1,0) = 1.; f(1,1) = 0.;
}

void zero_f(const Vector &x, Vector &f){
    f(0) = 0.;
    f(1) = 0.;
}

double T_fun(const double &salinity){
    return -(0.6037*salinity + 0.00058123*pow(salinity, 3));
}

double delta_c_s_fun(const double &temperature, const double &salinity){
    return -0.00313*salinity -
           0.00704*temperature -
           0.0000783*salinity*temperature +
           16.9*salinity*pow(temperature, -2);
}
