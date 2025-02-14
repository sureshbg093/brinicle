#include "header.h"

Config::Config(bool master, int nproc):
    master(master),
    nproc(nproc)
{}

Artic_sea::Artic_sea(Config config):
    config(config),
    pmesh(NULL), fec(NULL), fec_v(NULL), fespace(NULL), fespace_v(NULL),
    block_true_offsets(3),
    theta(NULL), phi(NULL), 
    w(NULL), psi(NULL),
    v(NULL), phase(NULL), 
    V(NULL),
    cond_oper(NULL), flow_oper(NULL),
    ode_solver(NULL), cvode(NULL), arkode(NULL),
    paraview_out(NULL)
{}

void Artic_sea::run(const char *mesh_file){
    //Run the program
    make_grid(mesh_file);
    assemble_system();
    for (iteration = 1, vis_iteration = 1; !last; iteration++, vis_iteration++)
        time_step();
    output_results();
}

Conduction_Operator::~Conduction_Operator(){
    //Delete used memory
    delete m_theta;
    delete m_phi;
    delete k_theta;
    delete k_phi;
    delete t_theta;
    delete t_phi;
}

Flow_Operator::~Flow_Operator(){
    delete f;
    delete g;
    delete m;
    delete d;
    delete c;
    delete ct;
    delete M;
    delete D;
    delete C;
    delete Ct;
}

Artic_sea::~Artic_sea(){
    delete pmesh;
    delete fec;
    delete fec_v;
    delete fespace;
    delete fespace_v;
    delete theta;
    delete phi;
    delete w;
    delete psi;
    delete v;
    delete phase;
    delete V;
    delete cond_oper;
    delete flow_oper;
    delete ode_solver;
    delete paraview_out;
    if (config.master) cout << "Memory deleted \n";
}
