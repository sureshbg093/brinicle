#include "header.h"

void Artic_sea::output_results(){
    if (config.master){
    //Print general information of the program
        cout << "\n\nSize: " << size << "\n"
             << "Mesh side: " << h_min << "\n"
             << "Serial refinements: " << serial_refinements << "\n"
             << "Parallel refinements: " << config.refinements - serial_refinements << "\n"
             << "Total refinements: " << config.refinements << "\n"
             << "Total iterations: " << iteration << "\n"
             << "Final mean convergence error: " << total_error/iterations_error << "\n";
    }
}
