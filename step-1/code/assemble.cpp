#include "header.h"

double rhs(const Vector &x){
    double r = sqrt(pow(x(0),2)+pow(x(1),2));
    double z = x(2);
    double f = (2 - out_rad/r)*(3*height - 2*z)*pow(z,2) +
               3*(r - 2*out_rad)*(height - 2*z)*r;
    return f;
}

double exact(const Vector &x){
    double r = sqrt(pow(x(0),2)+pow(x(1),2));
    double z = x(2);
    double f = 0.5*(2*out_rad - r)*r*(3*height - 2*z)*pow(z,2);
    return f;
}

double g(const Vector &x){
    double z(x(2));

    return (out_rad - int_rad)*pow(z,2)*(height - z);
}

void Artic_sea::assemble_system(){
    //Set the boundary values
    Array<int> ess_tdof_list;
    Array<int> nbc_marker;
    if (pmesh->bdr_attributes.Size()){
        //dirchlet(essential) boundary conditions
        Array<int> ess_bdr(pmesh->bdr_attributes.Max());
        ess_bdr = 1;
        ess_bdr[12] = 0;
        fespace->GetEssentialTrueDofs(ess_bdr, ess_tdof_list);

        //Neumann boundary conditions
        nbc_marker.SetSize(pmesh->bdr_attributes.Max());
        nbc_marker = 0;
        nbc_marker[12] = 1;
    }

    //Define biliniar form
    a = new ParBilinearForm(fespace);
    ConstantCoefficient one(1.);
    a->AddDomainIntegrator(new DiffusionIntegrator(one));
    a->Assemble();

    //Create RHS
    b = new ParLinearForm(fespace);
    FunctionCoefficient gcoeff(g);
    b->AddBoundaryIntegrator(new BoundaryLFIntegrator(gcoeff), nbc_marker);
    FunctionCoefficient f(rhs);
    b->AddDomainIntegrator(new DomainLFIntegrator(f));
    b->Assemble();

    //Define solution x
    x = new ParGridFunction(fespace);
    u = new FunctionCoefficient(exact);
    x->ProjectCoefficient(*u);

    //Create the linear system Ax=B
    a->FormLinearSystem(ess_tdof_list, *x, *b, A, X, B);
}
