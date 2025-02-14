#include "header.h"

//Rotational functions
void zero_f(const Vector &x, Vector &f);
double r_f(const Vector &x);
void rot_f(const Vector &x, DenseMatrix &f);
void r_inv_hat_f(const Vector &x, Vector &f);

void Artic_sea::assemble_system(){
    //Initialize the system
    t = 0;
    dt = config.dt_init;
    last = false;
    vis_steps = config.vis_steps_max;
    vis_impressions = 0;

    //Define variables
    theta = new ParGridFunction(fespace);
    w = new ParGridFunction(fespace);
    psi = new ParGridFunction(fespace);
    v = new ParGridFunction(fespace_v);

    Theta = new HypreParVector(fespace);
    W = new HypreParVector(fespace);
    Psi = new HypreParVector(fespace);
    V = new HypreParVector(fespace_v);

    //Initialize operators
    cond_oper = new Conduction_Operator(config, *fespace, *fespace_v, dim, pmesh->bdr_attributes.Max(), *Theta);
    flow_oper = new Flow_Operator(config, *fespace, *fespace_v, dim, pmesh->bdr_attributes.Max(), *Theta);

    //Solve initial velocity field
    flow_oper->Solve(*W, *Psi, *V);

    //Read initial global state
    theta->Distribute(Theta);
    w->Distribute(W);
    psi->Distribute(Psi);
    v->Distribute(V);

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
        ode_solver->Init(*cond_oper);
    else if (cvode){
        cvode->Init(*cond_oper);
        cvode->SetSStolerances(config.reltol_sundials, config.abstol_sundials);
        cvode->SetMaxStep(dt);
        cvode->SetStepMode(CV_ONE_STEP);
        ode_solver = cvode;
    }
    else if (arkode){
        arkode->Init(*cond_oper);
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
    paraview_out->RegisterField("Vorticity", w);
    paraview_out->RegisterField("Stream", psi);
    paraview_out->RegisterField("Velocity", v);
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

void zero_f(const Vector &x, Vector &f){
    f(0) = 0.;
    f(1) = 0.;
}

double r_f(const Vector &x){
    return x(0);
}

void rot_f(const Vector &x, DenseMatrix &f){
    f(0,0) = 0.; f(0,1) = -1.;
    f(1,0) = 1.; f(1,1) = 0.;
}

void r_inv_hat_f(const Vector &x, Vector &f){
    f(0) = pow(x(0) + epsilon_r, -1);
    f(1) = 0.;
}
