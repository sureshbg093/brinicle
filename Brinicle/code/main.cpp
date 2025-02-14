#include "header.h"

//Dimensions of the mesh
double Rmin;
double Zmin;
double Rmax;
double Zmax;

//Simulation parameters
double epsilon_r;
double c_l;

//Brinicle conditions
double vel, r0;
double theta_in, theta_out;
double phi_in, phi_out;
double n_l, n_h;
double theta_n, phi_n;

int main(int argc, char *argv[]){
    //Define MPI parameters
    int nproc = 0, pid = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    //Define program paramenters
    const char *mesh_file;
    Config config((pid == 0), nproc);
    int nDeltaT, nEpsilon_eta, nEpsilon_r;

    OptionsParser args(argc, argv);
    args.AddOption(&mesh_file, "-m", "--mesh",
                   "Mesh file to use.");
    args.AddOption(&Rmin, "-Rmin", "--Rmin",
                   "Minimum R border.");
    args.AddOption(&Rmax, "-Rmax", "--Rmax",
                   "Maximum R border.");
    args.AddOption(&Zmin, "-Zmin", "--Zmin",
                   "Minimum Z border.");
    args.AddOption(&Zmax, "-Zmax", "--Zmax",
                   "Maximum Z border.");

    args.AddOption(&config.dt_init, "-dt", "--time_step",
                   "Initial time step.");
    args.AddOption(&config.t_final, "-t_f", "--t_final",
                   "Final time.");
    args.AddOption(&config.vis_steps_max, "-v_s", "--visualization_steps",
                   "Visualize every n-th timestep.");

    args.AddOption(&config.refinements, "-r", "--refinements",
                   "Number of total uniform refinements.");
    args.AddOption(&config.order, "-o", "--order",
                   "Finite element order (polynomial degree) or -1 for isoparametric space.");
    args.AddOption(&config.ode_solver_type, "-ode", "--ode_solver",
                   "ODE solver: 1 - Backward Euler, 2 - SDIRK2, 3 - SDIRK3, \n"
                   "            11 - Forward Euler, 12 - RK2, 13 - RK3 SSP, 14 - RK4.");
    args.AddOption(&config.abstol_conduction, "-abstol_c", "--tolabsoluteConduction",
                   "Absolute tolerance of Conduction.");
    args.AddOption(&config.reltol_conduction, "-reltol_c", "--tolrelativeConduction",
                   "Relative tolerance of Conduction.");
    args.AddOption(&config.iter_conduction, "-iter_c", "--iterationsConduction",
                   "Iterations of Conduction.");
    args.AddOption(&config.abstol_sundials, "-abstol_s", "--tolabsoluteSUNDIALS",
                   "Absolute tolerance of SUNDIALS.");
    args.AddOption(&config.reltol_sundials, "-reltol_s", "--tolrelativeSUNDIALS",
                   "Relative tolerance of SUNDIALS.");

    args.AddOption(&config.T_f, "-T_f", "--temperature_fusion",
                   "Fusion temperature of the material.");
    args.AddOption(&nDeltaT, "-DT", "--DeltaT",
                   "Temperature interface interval (10^(-n)).");
    args.AddOption(&config.c_l, "-c_l", "--c_l",
                   "Liquid volumetric heat capacity.");
    args.AddOption(&config.c_s, "-c_s", "--c_s",
                   "Solid volumetric heat capacity.");
    args.AddOption(&config.k_l, "-k_l", "--k_l",
                   "Liquid thermal conductivity.");
    args.AddOption(&config.k_s, "-k_s", "--k_s",
                   "Solid thermal conductivity.");
    args.AddOption(&config.D_l, "-D_l", "--D_l",
                   "Liquid diffusion constant.");
    args.AddOption(&config.D_s, "-D_s", "--D_s",
                   "Solid diffusion constant.");
    args.AddOption(&config.L, "-L", "--L",
                   "Volumetric latent heat.");
    args.AddOption(&nEpsilon_eta, "-e_eta", "--epsilon_eta",
                   "Value of constatn epsilon for (1-phi)^2/(phi^3 + epsilon)(10^(-n)).");
    args.AddOption(&nEpsilon_r, "-e_r", "--epsilon_r",
                   "Value of constant epsilon for 1/(r + epsilon) (10^(-n)).");

    args.AddOption(&vel, "-v", "--vel",
                   "Velocity of inflow.");
    args.AddOption(&r0, "-r0", "--entrance",
                   "Size of inflow window.");
    args.AddOption(&theta_in, "-Ti", "--Theta_in",
                   "Initial temperature.");
    args.AddOption(&theta_out, "-To", "--Theta_out",
                   "Inflow temperature.");
    args.AddOption(&phi_in, "-Si", "--Phi_in",
                   "Initial salinity.");
    args.AddOption(&phi_out, "-So", "--Phi_out",
                   "Inflow salinity.");
    args.AddOption(&n_l, "-nl", "--n_length",
                   "Nucleation length.");
    args.AddOption(&n_h, "-nh", "--n_heigth",
                   "Nucleation height.");
    args.AddOption(&theta_n, "-Tn", "--Theta_n",
                   "Nucleation temperature.");
    args.AddOption(&phi_n, "-Sn", "--Phi_n",
                   "Nucleation salinity.");

    //Check if parameters were read correctly
    args.Parse();
    if (!args.Good()){
        if (config.master) args.PrintUsage(cout);
        MPI_Finalize();
        return 1;
    }
    if (config.master) args.PrintOptions(cout);

    {
        c_l = config.c_l;
        config.invDeltaT = pow(10, nDeltaT);
        config.epsilon_eta = pow(10, -nEpsilon_eta);
        epsilon_r = pow(10, -nEpsilon_r);
        Artic_sea artic_sea(config);
        artic_sea.run(mesh_file);
    }

    MPI_Finalize();

    return 0;
}
