

#include <string>
#include <iostream>
#include <ostream>
#include <exception>
#include <stdexcept>

//! Prints usage information and message to a stream.
class Usage {
private:
	const std::string _name; /*!< The name of the program */
	const std::string _msg; /*!< A message that will be printed at the top of usage information */
public:
	Usage() = delete;
	//! Usage constructor takes a message to be printed and the
	//! program name as arguments.
	Usage(const std::string& programName, const std::string& msg) : _name(programName), _msg(msg) {}
	friend std::ostream& operator<<(std::ostream& st, const Usage& that);
};

//! Overloaded stream operator to easily print Usage.
std::ostream& operator<<(std::ostream& st, const Usage& that);

//! Used for bitwise operations to indicate program parameters.
typedef unsigned int flag_t;

//! Bitwise flag values for command line parameters.
enum Flag {
	all = 1, /*!< Give all search results. */
	best = 2, /*!< Give only the single best result. */
	probability = 4, /*!< Parse the corpus expecting probability
			   annotations. */
	simple = 8 /*!< Parse corpus without annotations. */
};

//! Holds command line argument data.
struct param_t {
	flag_t flags; /*!< Holds command line switches */
	unsigned int maxEditDistance; /*!< Maximum edit distance the
					beam search will use. */
	std::string corpusFilename; /*!< Filename of the corpus or
				      wordlist. */
	param_t() = delete;
//! Takes command line flags and maximum edit distance to build
//! program parameters.
	param_t(flag_t f,const unsigned int n);
};
//! Tries to build parameter data from command line argument string
//! (only the hyphen portion); throws on malformed input.
param_t parseCmdLineArgs(const std::string& flagstring);

//! Directly prints contents of any container to std::cout
template <template <typename, typename...> class container_T, class value_T, typename... args>
void printContainer(const container_T<value_T, args...>& v) {
	typedef container_T<value_T, args...> T;
	for(typename T::const_iterator i(v.cbegin()); i != v.cend(); ++i) {
		std::cout << *i << std::endl;
	} // for
} // printContainer
//! Main loop of the program. Takes input and prints suggestions for correction.
/*! \param suggest An instance of Suggest, used to find corrections
 *  for the input.
 \param params Command line arguments that were specified.
 This function continuously reads from std::cin and feeds the input to
 a Suggest instance. The output is printed immediatly to std::cout. If
 a single newline is input, only a newline is printed to std::cout and
 no action is performed. The loop ends on EOF or two newlines etc.

 The Suggest type is parametrized in its corpus parser type. For this
 reason, this function is templated and can work with any Suggest type.
*/ 
template <typename parser_T>
void loopSuggest(const Suggest<parser_T>& suggest, const param_t& params) {
	for(std::string line; std::getline(std::cin, line);) {
		if(line.empty())
			break;
		if(params.flags & all) {
			printContainer(suggest.all(line));
		} else if(params.flags & best) {
			std::cout << suggest.best(line) << std::endl;
		} // else if best
		std::cout << std::endl;
	} // for
} // loopSuggest




