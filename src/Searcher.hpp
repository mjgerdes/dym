/* Searcher.hpp
   Marius Gerdes
   Matrikel Nr.:772451
*/

#include <vector>
#include <stack>
#include <memory>


#ifndef __SEARCHER_HPP__
#define __SEARCHER_HPP__

/*! Interface for modular beam-search in Tris
 *
 * This class provides an abstract interface to different kinds of
 *  beam-search algorithms that can be used on Tris (and possibly
 *  other structures). That means this class is meant to be derived
 *  from.
 The interface is implemented with the Curiously Reoccurring Template
 Pattern to provide compile time polymorphism, rather than using a full
 fledged abstract base class with virtual functions. This means that we
 save the runtime cost of a vtable lookup, with the downside that
 Searcher instances or child class instances can not be stored
 polymorphically in a container with a base class pointer (i.e. it is
 not a full sum type). Oh well...
 \param derived_T The type of a possible child class.
 \sa CorrectionSearcher

 \todo This is somewhat unnecessary since there is only one child
 class (CorrectionSearcher). I originally planned to try out different
 algorithms but this never happened.
*/
template <typename derived_T>
class Searcher {
public:
//! Empty struct to export the item type
/*! This structs serves as the wrapper for a typedef that allows
  access to the child class item type.
*/
	template <typename state_T>
	struct item {
		typedef typename derived_T::template item<state_T> type;
	};
//! Type of search-results.
	/*! Results found during searching are stored in a
	  hashmap. Its keys are the found strings, the value is a pair
	  with the number of the state the key was found in and, as the
	  second member of the pair, the amount of edit operations
	  needed to find the string. */
	typedef std::unordered_map<std::string, std::pair<unsigned int, unsigned int>> result_type;


protected:
//! Private Default constructor; this class should not be instantiated.
	Searcher() {}
	Searcher(const Searcher<derived_T>&) = delete;
public:
//! Returns initial item used to seed a search stack.
/*! \param startState The initial state to begin searching in.
  \param v A vector with indices being states and values state
  information.
  \return A pointer to an item representing the start of the
  beam-search.
*/
	template <typename state_T>
	std::unique_ptr<typename item<state_T>::type> initialItem(const unsigned int startState, const std::vector<std::unique_ptr<state_T>>& v) const {
		return static_cast<const derived_T*>(this)->initialItem(startState, v);
	}

//! Destructively inserts new search items into a stack.
/*! If no items are found, this function will not alter the stack.
  \param v A vector with indices being state numbers and values
  being state information.
  \param s The search stack to be manipulated; will only be pushed onto.
  \param i The item for which to push new items onto the stack. Usually
  the former top item.
*/
	template <typename state_T>
	void feedStack(std::stack<std::unique_ptr<typename item<state_T>::type> >& s, const std::vector<std::unique_ptr<state_T> >& v, const typename item<state_T>::type& i) {
		(static_cast<derived_T*>(this))->feedStack(s, v, i);
	}

//! Extract the results of a search.
/*! Searcher accumulates results during a call to feedStack. This
  method allows extraction of results.
  \return The unordered_map of results.
*/
	const result_type& getResults() const {
		return (static_cast<const derived_T*>(this))->getResults();
	}
};
	
#endif
