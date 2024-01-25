#include <iostream>
#include "data_base.h"
#include "fuel.h"
#include "material.h"
#include "flow.h"
#include "boundary_cond.h"

int main(int argc, char *argv[]) {
	std::string request;
	std::string base_name;

	if (argc > 2) {
		request = argv[1];
	} else {
		std::cerr << "Wrong command" << '\n';
        return 1;
	}

	base::Database database;

	if (request == "addf") {
          std::vector<std::string> file_names;/*({"nikap.csv", "nikat1.csv",
                                               "nikat2.csv", "nikat3.csv",
                                               "nikat4.csv", "nikat5.csv"});*/
          for (int i = 3; i < argc; ++i) {
          	file_names.push_back(argv[i]);
          }
          database.AddFuel(argv[2], file_names);
	} else if (request == "printf") {
		database.PrintFuel(argv[2], std::cout);
	} else if (request == "printm") {
		database.PrintMat(argv[2], std::cout);
	} else {
		std::cerr << "Wrong command" << '\n';
        return 1;
	}
    return 0;
}
