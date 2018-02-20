/* CorrectionSearcher.hpp
   Marius Gerdes
   Matrikel Nr.: 772451
*/

#include "Searcher.hpp"
#include <string>

#ifndef __CORRECTIONSEARCHER_HPP__
#define __CORRECTIONSEARCHER_HPP__


/*! Modular beam-search in tris for a given, possibly misspelled,
 *  input string.
 *
 *! As a child class of Searcher, CorrectionSearcher can be provided
 *  to the findWith member function of the Tri class to perform
 *  depth-first beam searches through the Tris word graph.
 * CorrectionSearcher requires an input string and a cutoff
 * edit-distance. The input key can be possibly misspelled, i.e. not a
 * word in the language described by the Tri. The CorrectionSearcher
 * traverses the Tri, trying to find strings that are actually keys in
 * the Tri and close to the input string. Closeness of strings is
 * measured through a number of unit edit-operations that are necessary
 * to turn one string into another. The cutoff edit-distance threshold is
 * used to prune the search tree.
 * CorrectionSearcher accumulates strings that can be reached through
 * these edit operations as its results, which must be extracted with
 * getResults.
 * 
 * Although the search is realized through a stack, CorrectionSearcher
 * does not maintain its own stack. This, as well as the iteration, has
 * to be provided by the user of CorrectionSearcher.
 * 
 * Results (\cword result_type ) is an unordered_map with the winning candidate
 * strings as keys and values as pairs of corresponding edit distance and
 * state number (for value extraction in the Tri). 
 * 
 * \todo This class is somewhat needlessly templated. I originally
 * introduced the state_T type parameter because the State type of Tri is
 * private and CorrectionSearcher should not have to know about it - or
 * it might change. It has become apparent that Tri::State will not
 * change, so we could either make CorrectionSearcher a friend or make
 * State public or put it into global scope. Either way, without the
 * templates we would have simpler types and smaller headers. Refactoring!
 * 
 * \todo unique_ptr and excessive copying
 * Throughout this class we use ubiquitous unique_ptrs and pretty much
 * everything is const and being copied rather than mutated. This was
 * intentional, because when writing this class I was paranoid about
 * memory leaks and the complexity of mutable state. Now that it seems to
 * work, this might be refactored out because:
 *  - the copying might simply be unnecessary operations in a perfomance
 *  critical inner loop (might be mute point due to optimization)
 *  - Having a stack of unique_ptr to items might cause cache misses,
 *  because memory is not laid out continuously on the heap.
 * UPDATE: Profiling has revealed neither of these considerations to be
 * significant (or I have misunderstood the results)
 */
class CorrectionSearcher : public Searcher<CorrectionSearcher> {
public:
//! Data tuples that are put on the stack to perform the search.
	/*! Items are used to incrementally compute edit distance
	  between certain strings, rather than calculating the edit
	  distance between two given strings monolithically over and
	  over again.
	  To do this, they carry a count of edit operations performed, a
	  candidate string that is being incrementally concatenated in
	  certain edit operations and a pointer into the original input
	  string, representing roughly the amount of the string consumed
	  and the next input character.
	  Different edit operations are distinguished (almost) only by wether or
	  not they advance the pointer into the input string (nextChar)
	  or what they append to the candidate string.
	*/
	template<typename state_T>
	struct item {
		const state_T& stateData; /*!< Contains transition and
					    end state information. */
		const unsigned int state; /*!< The current state name
					    (as a number). */
		const unsigned int editDistance; /*!< Number of edit
						  * operations necessary for this item to have been
						  * produced. */
		std::string::const_iterator nextChar; /*!< Iterator
						       * pointing to the next char in the original input
						       * string. */
		std::string candidate; /*!< The correction candidate
					* that has been built so far. */

//! Empty items make no sense.
		item() = delete;
		//! No copies allowed.
		item(const item&) = delete;
//! To construct an item, all its member fields must be fully provided
//! with values. These are guaranteed not to be changed.
		item(const state_T& newStateData, const unsigned int newState, const unsigned int newEditDistance,
		     std::string::const_iterator newNextChar, std::string newCandidate)
			: stateData(newStateData), state(newState), editDistance(newEditDistance), nextChar(newNextChar), candidate(newCandidate) {}
	};
private:
	const std::string _word; /*!< The original input string,
				   possibly misspelled. items point
				   into this with nextChar . */
	const unsigned int _cutoffDistance; /*!< The maximum number of
					      edit operations for any item to be legal. */
	result_type _results; /* Successful corrections and their data
			       * are stored here. */

private:
	//! Creates item representing no-operation or successful
	//! transition over input string.
	template <typename state_T>
	std::unique_ptr<item<state_T>> _noopItem(const unsigned char transChar, const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const;

//! Creates item representing the substitution of a character in the
//! input string by another character.
	template <typename state_T>
	std::unique_ptr<item<state_T>> _substitutionItem(const unsigned char transChar, const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const;

//! Creates item representing the insertion of a new character at a
//! position in the input string.
	template <typename state_T>
	std::unique_ptr<item<state_T>> _insertionItem(const unsigned char transChar, const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const;

//! Creates item representing the deletion of a character at a
//! position in the input string.
	template <typename state_T>
	std::unique_ptr<item<state_T>> _deletionItem(const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const;
//! Creates item representing the swapping of two consecutive characters in the input string.
	template <typename state_T>
	std::unique_ptr<item<state_T>> _transpositionItem(const unsigned char thisWasPutLeft, const unsigned char thisWasPutRight, const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const;

//! Pushes certain edit operation items for a given character
//! transition in the current state on the stack.
	template <typename state_T>
	void _pushEditOperations(const unsigned char transChar, const unsigned int nextState, const item<state_T>& top, std::stack<std::unique_ptr<item<state_T>>>& s, const std::vector<std::unique_ptr<state_T>>& v) const;

public:
//! Can't correct nothing!
	CorrectionSearcher() = delete;
//! Makes no sense.
	CorrectionSearcher(const CorrectionSearcher&) = delete;
//! A CorrectionSearcher always requires a word to be corrected and a maximum edit distance to search.
	CorrectionSearcher(const std::string w, const unsigned int cutoff) : _word(w), _cutoffDistance(cutoff), _results() {}

//! Creates the initial item to seed a stack for further searching.
	template <typename state_T>
	std::unique_ptr<item<state_T>> initialItem(const unsigned int startState, const std::vector<std::unique_ptr<state_T>>& v) const;

//! \brief Given a transition vector and a top item, destructively feeds the
//! given stack with new search items based on the top item. Pushes
//! multiple items but does not loop.
	template <typename state_T>
	void feedStack(std::stack<std::unique_ptr<item<state_T>>>& s, const std::vector<std::unique_ptr<state_T> >& v, const item<state_T>& top); 
//! Extract possible correction strings found during search.
	inline const result_type& getResults() const;
};  // CorrectionSearcher


/* Since all member functions are templates, they could be defined in
 * the header. However, I like to seperate the definition from
 * declaration anyway (if only for indentation level); so that is what
 * the following is for. */

#define item CorrectionSearcher::item
#define result_type CorrectionSearcher::result_type

#include "CorrectionSearcher.tpp.hpp"

#undef result_type
#undef item

#endif
