#include "header.h"

//Dimensions of the mesh
double Rmin;
double Zmin;
double Rmax;
double Zmax;

//Parameters of the simulation
double alpha;
int  Mterms = 6;
int  Nterms = 6;
std::vector<double> Coeficients(Mterms*Nterms ,0.0);

int main(int argc, char *argv[]){
    //Define MPI parameters
    int nproc = 0, pid = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    //Define program paramenters
    const char *mesh_file;
    Config config((pid == 0), nproc);

    OptionsParser args(argc, argv);
    args.AddOption(&mesh_file, "-m", "--mesh",
                   "Mesh file to use.");
    args.AddOption(&Rmin, "-Rmin", "--Rmin",
                   "Minimum R border");
    args.AddOption(&Rmax, "-Rmax", "--Rmax",
                   "Maximum R border");
    args.AddOption(&Zmin, "-Zmin", "--Zmin",
                   "Minimum Z boder");
    args.AddOption(&Zmax, "-Zmax", "--Zmax",
                   "Maximum Z boder");
    args.AddOption(&config.order, "-o", "--order",
                   "Finite element order (polynomial degree) or -1 for isoparametric space.");
    args.AddOption(&config.refinements, "-r", "--refinements",
                  "Number of total uniform refinements.");
    args.AddOption(&alpha, "-a", "--alpha",
                   "Alpha coefficient for the material.");
    args.AddOption(&config.dt_init, "-dt", "--time_step",
                   "Initial time step.");
    args.AddOption(&config.t_final, "-t_f", "--t_final",
                   "Final time.");
    args.AddOption(&config.vis_steps_max, "-v_s", "--visualization_steps",
                   "Visualize every n-th timestep.");
    args.AddOption(&config.ode_solver_type, "-ode", "--ode_solver",
                   "ODE solver: 1 - Backward Euler, 2 - SDIRK2, 3 - SDIRK3, \n"
                   "            11 - Forward Euler, 12 - RK2, 13 - RK3 SSP, 14 - RK4.");
    args.AddOption(&config.reltol, "-reltol", "--tolrelativaSUNDIALS",
                   "Tolerancia relativa de SUNDIALS solvers");
    args.AddOption(&config.abstol, "-abstol", "--tolabsolutaSUNDIALS",
                   "Tolerancia absoluta de S");

    //Check if parameters were read correctly
    args.Parse();
    if (!args.Good()){
        if (config.master) args.PrintUsage(cout);
        MPI_Finalize();
        return 1;
    }
    if (config.master) args.PrintOptions(cout);

    //Calculate coeficients of the exact solution
    Calc_Coe(Zmax, Rmax, Coeficients);

    //Print solution parameters
    if (config.master) {
      print_exact();
      print_initial();
    }

    //Run the program for different refinements
    tic();
    Artic_sea artic_sea(config);
    artic_sea.run(mesh_file);

    MPI_Finalize();

    return 0;
}
