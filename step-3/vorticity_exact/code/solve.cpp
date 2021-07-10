#include "header.h"

void rot_f(const Vector &x, DenseMatrix &f);

void Artic_sea::solve_system(){

    //Create the complete bilinear operator:
    //
    //   H = [ M    C ] 
    //       [ C^t  D ] 
    Array2D<HypreParMatrix*> hBlocks(2,2);
    hBlocks = NULL;
    hBlocks(0, 0) = M;
    hBlocks(0, 1) = C;
    hBlocks(1, 0) = C->Transpose();
    hBlocks(1, 1) = D;

    Array2D<double> blockCoeff(2,2);
    blockCoeff(0, 0) = 1.;
    blockCoeff(0, 1) = -1.;
    blockCoeff(1, 0) = -1.;
    blockCoeff(1, 1) = -1.;

    HypreParMatrix *H = HypreParMatrixFromBlocks(hBlocks, &blockCoeff);

    SuperLUSolver *superlu = new SuperLUSolver(MPI_COMM_WORLD);
    Operator *SLU_A = new SuperLURowLocMatrix(*H);
    superlu->SetOperator(*SLU_A);
    superlu->SetPrintStatistics(true);
    superlu->SetSymmetricPattern(true);
    superlu->SetColumnPermutation(superlu::PARMETIS);
    superlu->SetIterativeRefine(superlu::SLU_DOUBLE);

    //Solve the linear system Ax=B
    X.Randomize();
    superlu->Mult(B, X);

    //Recover the solution on each proccesor
    w->Distribute(&(X.GetBlock(0)));
    for (int ii = 0; ii < w->Size(); ii++)
        (*w)(ii) += (*w_aux)(ii); 
    
    psi->Distribute(&(X.GetBlock(1)));
    for (int ii = 0; ii < psi->Size(); ii++)
        (*psi)(ii) += (*psi_aux)(ii); 

    v = new ParGridFunction(fespace_v);
    GradientGridFunctionCoefficient psi_grad(psi);
    MatrixFunctionCoefficient rot(dim, rot_f);
    MatrixVectorProductCoefficient rV(rot, psi_grad);
    v->ProjectCoefficient(rV);

    //calculate solution error
    FunctionCoefficient w_exact(exact_w);
    FunctionCoefficient psi_exact(exact_psi);
    actual_error_w = w->ComputeL2Error(w_exact);
    actual_error_psi = psi->ComputeL2Error(psi_exact);

    //Delete used memory
    delete H;
    delete superlu;
    delete SLU_A;
}

void rot_f(const Vector &x, DenseMatrix &f){
    f(0,0) = 0.; f(0,1) = -1.;
    f(1,0) = 1.; f(1,1) = 0.;
}
