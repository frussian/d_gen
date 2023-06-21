#include <iostream>
#include <stdexcept>
#include <fstream>

#include "d_gen/BuildError.h"
#include "d_gen/DGen.h"

char *prog_path = nullptr;
std::optional<int> seed;
std::optional<int> tests_num;

void parse_args(int argc, char *argv[]) {
	for (int i = 1; i < argc; i++) {
		switch (argv[i][1]) {
			case 'f':
				prog_path = argv[i]+2;
				break;
			case 's':
				seed = std::atoi(argv[i]+2);
				break;
			case 'n':
				tests_num = std::atoi(argv[i]+2);
				break;
			default:
				std::cout << "warning: unknown parameter " << argv[i][1] << std::endl;
				break;
		}
	}
}

void print_usage(char *this_prog) {
	std::cout << "usage: " << this_prog << " -f<path to program> -s<optional seed> -n<tests_num>" << std::endl;
}

int main(int argc, char *argv[]) {
	parse_args(argc, argv);

	if (!prog_path || !tests_num.has_value()) {
		print_usage(argv[0]);
		return 0;
	}

	DGen::init_backend();

	try {
		std::ifstream stream;
		stream.open(prog_path);
		if (stream.fail()) {
			throw std::runtime_error("can't read file");
		}

		DGen d_gen(stream);
		std::string json = d_gen.generate_json(*tests_num, seed);

		std::cout << "generated tests:\n";
		std::cout << json << std::endl;
		stream.close();
	} catch (const BuildError &err) {
		std::cout << "errors" << std::endl;
		for (const auto &e: err.errors) {
			std::cout << e.pos.line << ":" << e.pos.col << " " << e.msg << std::endl;
		}
	}
	catch (const std::exception &err) {
		std::cout << "error" << std::endl;
		std::cout << err.what() << std::endl;
	}

	return 0;
}
