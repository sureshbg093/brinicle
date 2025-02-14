#include "header.h"

void Artic_sea::output_results(){
    //Print general information of the program
    if (config.master)
        cout << "\n\nSize: " << size << "\n"
             << "Serial refinements: " << serial_refinements << "\n"
             << "Parallel refinements: " << config.refinements - serial_refinements << "\n"
             << "Total refinements: " << config.refinements << "\n"
             << "Total iterations: " << iteration << "\n"
             << "Total printing: " << vis_impressions << "\n";
}
