/* SimpleCorpusParser.hpp
   Marius Gerdes
   Matrikel Nr.: 772451
*/

#include "CorpusParser.hpp"

#ifndef __SIMPLECORPUSPARSER_HPP__
#define SIMPLECORPUSPARSER_HPP__



class SimpleCorpusParser : public CorpusParser<SimpleCorpusParser> {
private:
	CorpusParserData _data; /*!< member variables and error
				  messages */ 
	double _defaultDouble; /*! default double value to construct
				*  pairs with; needed since corpus is
				*  not annotated with such values. */

public:
	SimpleCorpusParser() = delete;
	SimpleCorpusParser(const SimpleCorpusParser&) = delete;
/*! Constructs object to parse simple corpora. The filename is used
  only in error messages.
  \param corpusFilename Filename for error messages
*/
	SimpleCorpusParser(const std::string& filename)
		: _data(filename), _defaultDouble(1.0) {}
/*! Constructs a pair of values from a single line of input text.

  Functions similarly to ProbabilityCorpusParser::parseLine but expects
  a different format; lines should be of the form .+ , where
  . is any alphanumeric character.
  This function throws \cword ParseError if a malformed line is passed
  to parseLine.
  The return value is meant for use in Tri<double>::insert

  \param line A string to construct a pair of values for.
  \return A pair of a word and a double=1.0

  \sa ProbabilityCorpusParser::parseLine

  \todo Returning a default probability of 1.0 is wasteful and a hack,
  done only because other classes (Tri and Suggest) expect it. Suggest
  could be partially specialized for SimpleParser to not require a
  probability at all. Then, this function would also change and return std::string.
*/
	const 	std::pair<const std::string, const double> parseLine(const std::string& line) {
		++_data.linesConsumed;

		for(std::string::const_iterator i(line.cbegin()); i != line.cend(); ++i) {
			if(!isalpha(*i)) {
				_data.parseError("Encountered unexpected '" + std::string(i,i) + "' while trying to parse a word.");
			} // if

		} // for

		return std::make_pair(std::string(line.cbegin(), line.cend()),
				      _defaultDouble);
	} // parseLine
		
	unsigned int lineNumber() {
		return _data.linesConsumed;
	}
}; // SimpleCorpusParser


#endif
