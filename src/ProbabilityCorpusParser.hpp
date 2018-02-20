/* ProbabilityCorpusParser.h
   Marius Gerdes
   Matrikel Nr.: 772451
*/

#include "CorpusParser.hpp"

#ifndef __PROBABILITYCORPUSPARSER_HPP__
#define __PROBABILITYCORPUSPARSER_HPP__

/*! Constructing values from lines in probability annotated word list
 *  corpora.

 Instances of this class can use the \cword parseLine method to
 construct pairs of values from single lines of probability annotated
 word list corpora. Filenames provided to this class are used only in
 error messages. Instances may throw exceptions of type \cword
 ParseError when reading lines, but not on construction.

 \sa CorpusParser
*/
class ProbabilityCorpusParser : public CorpusParser<ProbabilityCorpusParser> {
private:
	CorpusParserData _data; /*!< Member variables and error
				 * throwing. */

public:
	ProbabilityCorpusParser() = delete;
	ProbabilityCorpusParser(const ProbabilityCorpusParser&) = delete;
/*! Creates a new parser for a word corpus with probability
 *  annotations. This class does not actually open any files, the
 *  filename is used only for throwing errors during parsing.
 \param corpusFilename Filename of the wordlist corpus to be
 parsed. Only used for error messages.
*/
	ProbabilityCorpusParser(const std::string corpusFilename) : _data(corpusFilename) {}

/*! Returns parsed data for one line of input.

  This function does not read from a file or a stream; it only takes a
  string, supposed to be one line of the input. The function
  tracks internally how many lines of input were consumed, to
  produce informative error messages.
  Lines are of the form .+'TAB'('-' | [0-9]+)'.'[0-9]+ where . is any
  alphanumeric character.  
  This function will happily throw when an invalid string is passed to
  it. The type of exception is \cword ParseError .
  The return type is a pair, intended to be used in Tri<double>::insert
  (boxing/unboxing is hopefully optimized away).
  \param line One line of input from a corpus file.
  \return A pair consisting of the word and its probability annotation
  value.

  \todo This cannot deal with unicode :(
*/
	const std::pair<const std::string, const double> parseLine(const std::string& line) {
		++_data.linesConsumed;
// we use an ad-hoc finite state transfuser to build the pair.
		enum State {
			wordParse, /*!< State in which characters of
				     words are expected */
			doubleParse, /*!< state in which digits are
				       expected */
			preDot, /*!< state in which a hyphen or a
				  digit is expected, before the dot of a
				  floating point number */
			atLeastOneNumber, /*!< state in which one or
					    more numbers are expeceted (before the dot but
					    after a possible hyphen) */
			number /*!< state in which any digit is
				 expected, or end of input */
		};


// wordparse is the starting state
		State state(wordParse);
// two iterators that will point to the string positions where the
// word ends or the double string representation starts, to construct
// their values later from a substring.
		std::string::const_iterator wordEnd(line.cend()), doubleStart(line.cend());
// the iterator used to consume the string, declared outside of
// for-loop scope because we need it later
		std::string::const_iterator i(line.cbegin());
		for(; i != line.cend(); ++i) {
			switch(state) {
			case wordParse:
				if(isalpha(*i)) {
					break; /// \todo this case can
					       /// be subsumed
				} else if(*i == (char)9) {
// we encountered a tab character
					wordEnd = i;
					doubleStart = i + 1;
					state = doubleParse;
				} else {
					_data.parseError("Encountered unexpected '" + std::string(i, i) + "' while trying to parse a word.");
				} // else
				break; // wordParse
			case doubleParse:
				if(*i == '-') {
					state = atLeastOneNumber;
					break;
				}
				// select case fallthrough
			case atLeastOneNumber:
				if(!isdigit(*i)) {
					_data.parseError("Malformed floating point number.");
				}
				state = preDot;
				break;
			case preDot:
				if(*i == '.') {
					state = number;
					break;
				} else if(!isdigit(*i)) {
					_data.parseError("Malformed floating point number.");
				}
				break;
			case number:
				if(!isdigit(*i)) {
					_data.parseError("Malformed floating point number.");
				}
				break;
			} // switch
		} // for
// this will hopefully invoke the move constructor
//! \todo stod is too lax for my taste (fails silently sometimes), so we do
//verification of the double ourselves; it occurred to me that this
//could be done simply using stringstreams and the >> operator... oh well

		if(doubleStart == line.cend() || wordEnd == line.cend()) {
			_data.parseError("Malformed input.");
		}

		return std::make_pair(std::string(line.cbegin(), wordEnd),
				      stod(std::string(doubleStart, i)));
	} // function
//! Gives the number of lines consumed.	
	unsigned int lineNumber() const {
		return _data.linesConsumed;
	}
}; // class 


#endif
