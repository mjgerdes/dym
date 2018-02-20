/* main.cpp
   Marius Gerdes
   Matrikel Nr.: 772451
*/


#include "Suggest.hpp"
#include <exception>
#include <memory>
#include <string>
#include "IO.hpp"


int main(int argc, char** argv) {
	try {
		std::string args("");
		switch(argc) {
		case 1:
			std::cout << IO::Usage(argv[0], "No parameters given.");
			return 0;
		case 2:
			args = "";
			break;
		case 3:
			args = argv[1];
			break;
		default:
			std::cerr << IO::Usage(argv[0], "Incorrect number of parameters.");
			return 1;
		} // switch
		
		IO::param_t params(IO::parseCmdLineArgs(args));
		params.corpusFilename = std::string(argv[argc - 1]);
		if(params.flags & IO::probability) {
			std::unique_ptr<ProbabilitySuggest> suggest(new ProbabilitySuggest(params.corpusFilename, params.maxEditDistance));
			IO::loopSuggest(*suggest, params);
		} else if(params.flags & IO::simple) {
			std::unique_ptr<SimpleSuggest> suggest(new SimpleSuggest(params.corpusFilename, params.maxEditDistance));
			IO::loopSuggest(*suggest, params);
		}
	} catch(ParseError& E) {
		// don't print usage info
		std::cerr << E.what() << std::endl;
	} catch(std::exception& E) {
		std::cerr << IO::Usage(argv[0], E.what());
	}
}
