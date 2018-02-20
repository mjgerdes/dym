/* Suggest.hpp
 *Marius Gerdes
 *Matrikel-Nr.: 772451
 */

#include "Tri.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include "ProbabilityCorpusParser.hpp"
#include "SimpleCorpusParser.hpp"
#include <algorithm>

#ifndef __SUGGEST_HPP__
#define __SUGGEST_HPP__

/*! Corpus based correction suggestions for misspelled strings.
 *
 * This class provides functionality to read in a wordlist corpus and
 * then give suggestions for the correction of single input words. A
 * maximum edit-distance can be specified to narrow or widen the range
 * of possible corrections. Corrections that have lower edit distance
 * or that have a higher probability in the internal wordlist will be
 * recieve prefering treatment by the two public member functions
 * \cword best and \cword all , which return the single best and all
 * sorted results respectively.

This class is templated. The type variable \cword corpusParser_T is
 * used to specify a type that provides the corpus parsing/reading
 * capability. This class will throw, if the \cword CorpusParser_T
 * type instance throws on an invalid corpus file.
Two convenience typedefs exist to hide this type variable: \cword
 * ProbabilitySuggest and \cword SimpleSuggest.
They provide suggestions for corpora annotated with probabilities and
 * without any annotations, respectively.

\todo Wasteful because probabilities are still compared, even when the
corpus actually does not contain annotations. This could be fixed with
some refactoring and partial template specialization on the nested
type parameter given to the CorpusParser; see the CorpusParser
class for more.
*/
template <typename corpusParser_T>
class Suggest {
private:
	const std::string _corpusFilename; /*!< The filename of the
					     corpus text file that is used to find a correct spelling
					     suggestion. */
	Tri<double> _words; /*!< A Tri that is used to store the
			     * correct spelling; it is searched to
			     * find suggestions. Parametrized with
			     * double to store possible word probabilities.*/
	unsigned int _maxEditDistance; /*!< The maximum edit
					* operations that will be performed to find
					* a correction suggestion. */

private:

//! Convenience typedef
		typedef Tri<double>::TolerantResult T;

//! A Function object to create a total ordering of search results.
/*! This is used in 'all' to sort the search results and in 'best' to
	find the best result. To create a total order, edit distance
	and probability is compared. Edit distanced is weighed more
	than Probability.
*/
	struct _cmpTolerantResult {
		bool operator()(const T& p1, const T& p2) {
			if(p1.editDistance == p2.editDistance) {
				return p1.second < p2.second;
			} else {
				return p1.editDistance < p2.editDistance;
			}
		} // operator()
	}; // _cmp_tolerantResult
/*! Uses a \cword CorpusParser to verify and read-in a corpus.

This function takes an input filestream and processes the entire file,
	filling in the \cword _cword member Tri with appropriate
	values (in this case key strings for words and \cword double
	for values).
Throws on invalid corpus file format, which is verified by the 
\cword CorpusParser instance.
\param corpus An input file stream of the corpus to be processed.
*/
	void _readCorpus(std::ifstream& corpus) {
		corpusParser_T corpusParser(_corpusFilename);
		std::string line;
		while(corpus.good()) {
			std::getline(corpus, line);
// empty lines are ignored
			if(!line.empty()) {
				_words.insert(corpusParser.parseLine(line));
			}
		} // while
	} // _readCorpus
		
public:
	Suggest() = delete;
	Suggest(const Suggest&) = delete;
/*! Creates a new Suggest instance from a Corpus and a maximum edit
 *  distance.

 * This function may throw on file reading errors or an invalid
 * formatting of the provided corpus.
\param corpusFileName Filename of the text corpus that will be read
	into the internal list of correct words.
\param maxEditDistance Maximum amount of edit operations for which
	suggestions will be made.
*/
	Suggest(const std::string& corpusFilename, const unsigned int maxEditDistance)
		: _corpusFilename(corpusFilename), _words(Tri<double>()), _maxEditDistance(maxEditDistance) {
		std::ifstream corpusFile(corpusFilename);

		if(!corpusFile.is_open()) {
			throw (std::runtime_error("error in Suggest: File '"
						  + corpusFilename
						  + "' could not be opened."));
		}
			
		_readCorpus(corpusFile);
		corpusFile.close();
	} // Suggest ctor
//! Overloaded constructor to read directly from a stream.
	Suggest(std::ifstream& corpus, const std::string& name, const unsigned int n)
		: _corpusFilename(name), _words(Tri<double>()), _maxEditDistance(n) {
		_readCorpus(corpus);
	} // Suggest ctor
/*! Finds the best correction suggestion for a given word.
 * 
 To find a best suggestion, the internal wordlist is searched for
	possible corrections. The results are then themselves searched
	for the best candidate, while candidates are better if they
	have, first, a lower edit distance and , second, a higher
	probability in the corpus. If the internal search yields no
	result, the empty string is returned.

\param w A word, possibly misspelled, for which suggestions should be
found.
\return The best correction suggestion that could be found or the
empty string.
*/
	std::string best(const std::string& w) const {
		const _cmpTolerantResult f; // comparison object to
					    // find the maximum
		auto v(_words.tolerantFind(w, _maxEditDistance));
		std::vector<T>::const_iterator winner(max_element(v.cbegin(), v.cend(), f));
		if(winner == v.cend())
// return empty string on no results
			return std::string("");
// otherwise return maximum element according to comparison function
		return std::move(winner->first);
	} // best
				
/*! Finds all possible correction suggestions.
 * 
 * This function searches the internal wordlist to find all possible
 * corrections for the input word and provided edit distance. The
 * results are returned as a sorted vector of strings. The vector is
 * sorted from best to worst; lower indices are better candidates. How
 * good a suggestion string is, is determined, first, by having a
 * lower edit distance and, second, by having a higher probability in
 * the internal wordlist.
 * If no results are found in the internal search, an empty vector is
 * returned.
 * \param w A word, possibly misspelled, for which correction suggestions
 * are to be found.
 * \return A vector of sorted suggestion strings.
 */
	std::vector<std::string> all(const std::string& w) const {
		const _cmpTolerantResult f;  // comparison object to
					     // sort the results
		auto v(_words.tolerantFind(w, _maxEditDistance));
		sort(v.begin(), v.end(), f);
// the returned vector has edit distance and probability in it, so we
// have to unpack it
		std::vector<std::string> v2;
		// std::sort puts lowest elements first, so we iterate
		// in reverse, since we want the best ones first
		for(auto i(v.crbegin()); i != v.crend(); ++i) {
			v2.push_back(i->first);
		}
		return std::move(v2);
	} // all
}; // class Suggest

/*! Convenience typedef to hide template parameter for Suggest classes
 *  that want to parse a corpus with probability annotations.
 */
typedef Suggest<ProbabilityCorpusParser> ProbabilitySuggest;
/*! Convenience typedef to hide template parameter for Suggest classes
 *  that want to parse a corpus with only words, no annotations. */
typedef Suggest<SimpleCorpusParser> SimpleSuggest;


#endif

