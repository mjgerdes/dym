/* CorpusParser.hpp
   Marius Gerdes
   Matrikel Nr.: 772451
*/

#include <string>
#include <exception>
#include <stdexcept>
#include <sstream>

#ifndef __CORPUSPARSER_HPP__
#define __CORPUSPARSER_HPP__


/*! Exception thrown when a malformed string is encountered during
 *  processing of the corpus.
 */
struct ParseError : public std::runtime_error {
	ParseError(const std::string what) : std::runtime_error(what) {}
}; // Parseerror

/*! Interface for types that parse lines of text from a word list
 *  corpus.

 This class only provides the abstract interface to CorpusParser
 types. It uses the CRT pattern to avoid the vtable lookup cost of
 virtual inheritance. It can not (and should not) be stored in
 polymorphic containers.
 A CorpusParserData class to abstract the member fields of child
 classes is also provided.
 \sa ProbabilityCorpusParser
 \sa SimpleCorpusParser
*/
template <typename derived_T>
class CorpusParser {
protected:
/*! Abstracts member fields and error throwing functionality for
  children of CorpusParser.

  This is implemented as a class declaration rather than member fields
  to be inherited because I think composition should be prefered
  over inheritance.
*/
	class CorpusParserData {
	public:
		const std::string corpusFilename; /*!< The filename of
						    the word list corpus that is being parsed; used only
						    for error messages. */
		unsigned int linesConsumed; /*!< The amount of lines
					     * that have been parsed
					     * thus far. */

	public:
		CorpusParserData() = delete;
		CorpusParserData(const CorpusParserData&) = delete;
/*! Constructs the member fields of a CorpusParser. */
		CorpusParserData(const std::string& filename) : corpusFilename(filename), linesConsumed(0) {}
/*! Used to throw errors when encountering a malformed string. */
		inline 	void parseError(const std::string& msg) {
			std::ostringstream message;
			message << corpusFilename << ", line:" << linesConsumed
				<< std::endl << msg;
			throw(ParseError(message.str()));
		} // parseError	
	}; // CorpusParserData



public:
/*! Constructs a pair of values from one line of input. 

  This function may throw \cword ParseError on a malformed string.
  \param line A line of text from a corpus.
  \return A pair of the read word and its probability in the corpus.
*/
	const std::pair<const std::string, const double> parseLine(const std::string& line) {
		return static_cast<derived_T>(*this).parseLine(line);
	}
/*! Returns a number indicating how many lines have been parsed.
  \return Number of lines consumed.
*/
	unsigned int lineNumber() const {
		return static_cast<const derived_T>(*this).lineNumber();
	}
}; // CorpusParser



#endif
