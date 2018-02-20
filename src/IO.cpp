
#include "Suggest.hpp"
#include "IO.hpp"


using namespace IO;

std::ostream& IO::operator<<(std::ostream& st, const Usage& that) {
	st << that._name << ": " << that._msg << std::endl <<
		"Usage: " << that._name << " [-abps] CORPUSFILE" << std::endl <<
		"Reads words from standard input and prints suggestions to standard output." << std::endl <<
		"Examples\n  Print all found suggestions, using probability based corpus and maximum edit distance 2:\n" <<
		that._name << " -apd2 corpus.txt" << std::endl <<
		"Options:\n" <<
		" -a, all - Return all found suggestions (default).\n" <<
		" -b, best - Return only the single best solution.\n" <<
		" -p, probability - Use tab seperated word corpus with floating point numbers to indicate word probability (default).\n" <<
		" -s, simple - Use a simple, non-probability corpus.\n" <<
		" -eN edit distance 0 <= N <= 9, Find suggestions with a maximum of N unit edit operations (default 1)." << std::endl;
	return st;
}
/*! Constructs an object representing command line option input. This
 *  constructor ensures that options passed to it are logically
 *  consistent and valid.
\param f Flags indicating program behaviour; checked for consistency
\param n Maximum edit distance 
*/
IO::param_t::param_t(flag_t f,const unsigned int n)
	: maxEditDistance(n), corpusFilename("") {
	f |= all;
	f |= probability;

// we will do either all or best, not both; all is default
	if(f & best)
		f &= ~all;
// same for probability and simple
	if(f & simple)
		f &= ~probability;
	flags = f;
}
/*! Validates a command line parameter string of flags (i.e. "-ape3")
 *  and, if valid, returns the corresponding param_t object. This
 *  function will throw on invalid input.
\param flagstring A string representing command line flags.
\return An object specifying program behaviour.
*/
param_t IO::parseCmdLineArgs(const std::string& flagstring) {

// validation is done with an ad-hoc finite state transfuser
	enum State {
		dash, parameters
	};



	State state(dash);
	flag_t flags(0);
// default max edit distance is 1
	unsigned int maxEditDistance(1);
// keep track how many parameters were parsed
	unsigned int paramCount(0);
	for(auto i(flagstring.cbegin()); i != flagstring.cend(); ++i) {
		switch(state) {
		case dash:
			if(*i != '-') {
				throw std::runtime_error("Malformed parameter list.");
			}
			state = parameters;
			break;
		case parameters:
			if(paramCount > 4) {
// user specified some option twice; be strict and terminate
				throw std::runtime_error("Too many flags.");
			};
			switch(*i) {
			case 'a':
				flags |= all;
				break;
			case 'b':
				flags |= best;
				break;
			case 's':
				flags |= simple;
				break;
			case 'p':
				flags |= probability;
				break;
			case 'e':
				++i;  // e must be followed by a
				      // number; advance input string
				if(i != flagstring.cend() && isdigit(*i)) {
					maxEditDistance = (unsigned int)std::stoi(std::string(i, i+1));
					break;
				}
// e was at end of string or was followed by a non-digit
				throw std::runtime_error("Bogus command line parameters.");

			default:
				throw std::runtime_error("Unrecognized command line parameter.");
			} // inner switch
			++paramCount;
			break;  // paramters
		} // switch
	} // for
	return param_t(flags, maxEditDistance);
}
