/* Tri.hpp
   Marius Gerdes
   Matrikel Nr.: 772451
*/

#include <map>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stack>
#include <iostream>
#include <string>
#include "CorrectionSearcher.hpp"

#ifndef __TRI_HPP__
#define __TRI_HPP__
/*! Trigraph Class to associate strings with values.

  This class provides functionality to associate string keys with
  values. It also allows the efficient and modular lookup of keys that are not the same as an
  actually stored key, but difer only a specified amount of unit edit
  operations from an actually stored key.
  Internally, the Tri uses a finite state automaton to find values
  associated with keys. The error-tolerant lookup is implemented using a
  depth-first beam search through the automaton.
  This is a template class. Tri is a polymorphic container that can be
  used to store any type. It is guaranteed not to modify or copy its
  elements, unless specifically requested (makeCopy). The type of stored
  elements is exported as \cword value_type.

  \todo This class only implements roughly the minimal functionality
  used in the program for lookup of misspelled word corrections. To be a
  full fledged container class, some things need to be added, like an
  iterator interface, ways to delete keys and clear the Tri. It would
  also be possible to have a type parameter not only for value types,
  but also for the type of key, i.e. not just std::string but anything
  that can be concatenated. 
  \todo unique_ptr is used internally to store States; this is probably
  a bad idea.
  \sa Suggest
*/
template <class value_T>
class Tri {
private:

/*! The type of indices into the state vector. */
	typedef unsigned state_T;

/*! The type of the data structure used to keep track of transitions
  between states. Keys are characters, values are names of states
  (unsigned int/state_T).

  std::map is implemented as a binary search tree (afaik); meaning we
  get O(log(n)) insertion and deletion - which is important
  while building the Tri on a corpus. std::map should also be
  good for iterating through the whole container, which we need
  to perform a beam search.
  This is the most performance critical choice of data structure for the
  whole program; according to my profiler, around 11% of
  execution time is spent in the <-comparison operator, used by
  map to find its elements. This is much more than any other
  component of the program.
  Note: I tried using std::unordered_map here since hashtables should
  have O(1) insertion. Profiling has revealed that std::map is
  actually faster (around x3). Could this be mitigated by
  providing my own hash function?
*/
	typedef std::map<unsigned char, state_T> transitions_T;
	/*! Represents states in the Tri

	  States themselves, although they are refered to using unsigned int
	  indices into the internal state vector, are not plain old data
	  types, since I decided to keep the transitions inside of them.
	*/
	struct State {
		bool endState; /*!< Encodes wether the state is a
				 final state in the Tri */
		transitions_T transitions; /*!< A map of transitions that lead, with some character, from this state to another. */
/*! Creates States. By default, a new State is not an end state, and
  has no transitions. */
		State() : endState(false), transitions() {};
/*! Here, like elsewhere, I delete the copy constructor because i want
  the compiler to warn me when I accidentally omit a &
  in a function parameter declaration. See copyFrom for
  State copying functionality. */
		State(const State&) = delete;
/*! To be in containers, types must provide either a copy or at least
  a move constructor. */
		State(State&& other) = default;
/*! Copy assignment operator is implemented using move semantics (note
  the pass-by-value). */
		State& operator=(State other) noexcept {
			std::swap(endState, other.endState);
			std::swap(transitions, other.transitions);
			return *this;
		} // operator=
		/*! Copying functionality.

		  Implemented as a named member function because I want to be able to
		  copy states explicitly - but not implicitly.
		  \param source The State to copy data from.
		  \return A reference to the State that data was copied to.
		*/
		State& copyFrom(const State& source) {
			endState = source.endState;
// this will call copy constructor of transitions
			transitions = source.transitions;
			return *this;
		} // copyFrom
	}; // State
/*! The States are kept in a vector; indices are the names of the
 *  states.

 \todo Why is this using unique_ptr? I think this was a bad choice that
 I made early on because I thought States need to be passed
 around. This turned out to be not the case. Now it is a hassle
 to change because other classes rely on this type signature -
 I should have used more typedefs.
 In any case the indirection through the pointers is unnecessary and
 may produce cache misses.
*/
	std::vector<std::unique_ptr<State>> _v;

/*! A jashmap of values that are stored in the Tri. Indices are the
  names of States (that are endSates). */
	std::unordered_map< state_T, value_T> _values;
private:
/*! Creates a new State and returns its name (uint). Suffixed with f
  to remind myself that this function is destructive/mutating
  state in the Tri: Every call to this function will construct
  and push a new State object onto the internal state vector.
  \return The name of the newly created State.
*/
	state_T _newStatef();

public:
/*! Creates an empty Tri with only one State */
	Tri();
/*! Deleted copy ctor to prevent accidental omissions of & - it would
  be a shame to accidentally copy a whole Tri (and possibly
  dangerous) */
	Tri(const Tri<double>&) = delete;
/*! Move Constructor */
	Tri(Tri<value_T>&& other) noexcept
	: _v(std::move(other._v)), _values(std::move(other._values)) {}
/*! Copy assignment operator, implemented with move semantics. 

  Since this is pass-by-value it will invoke the move ctor of other;
  values are then swapped into the assigned-to Tri. The source
  Tri will be destroyed when it goes out of scope - assignment
  will invalidate assigned-from objects. 
*/
	Tri<value_T>& operator=(Tri<value_T> other) noexcept {
		std::swap(_v, other._v);
		std::swap(_values, other._values);
	        return *this;
	}

/*! Exported type of stored values. */
	typedef value_T value_type;

/*! Convenience typedef for key/value pairs of the Tri */
	typedef std::pair<std::string, value_T> pair_type;

/*! The type of data returned by a call to \cword tolerantFind or
  \cword tolerantFindWith.

  This is a pod type; it exists to bundle not only key/value pairs of
  Strings found during a tolerant search but it also has the edit
  distance that was involved in finding the key.
*/
	struct TolerantResult {
		std::string first; /*!< A string found during a
				     tolerant search. */
		value_T second; /*!< The value associated with the
				  found key-string .*/
		unsigned int editDistance; /*!< The amount of edit
					     operations that were necessary to find the key-string .*/
		TolerantResult() = delete;
		TolerantResult(const TolerantResult&) = delete;
		TolerantResult(const TolerantResult&& other) noexcept
		: first(std::move(other.first)), second(std::move(other.second)), editDistance(std::move(other.editDistance)) {}
		TolerantResult& operator=(TolerantResult other) noexcept {
			std::swap(this->first, other.first);
			std::swap(this->second, other.second);
			std::swap(this->editDistance, other.editDistance);
			return *this;
		}
/*! TolerantResult constructor 
  \param s The string that was found in the Tri during a tolerant
  search.
  \param v The value associated with the string that was found.
  \param n The number of edit operations necessary to find the string.
*/
		TolerantResult(const std::string& s, const value_T& v, const unsigned int n)
			: first(s), second(v), editDistance(n) {}
	}; // TolerantResult

/*! Insert an object into the Tri and associate it with a given key.
  This function will update the Tri destructively. If a key is already
  present, it will be associated with the new value, the old one will be destroyed.
  \param key A string that will be associated with the provided object.
  \param value An object of type value_T, to be stored in the Tri.
*/
	void insert(const std::string key, const value_T& value);

/*! Insert key/value pair into the Tri.

  This function will update the Tri destructively.
  \param p A pair of a string and an object of type value_T.
*/
	void insert(const std::pair<const std::string, const value_T>& p);

/*! Batch-insert keys from a container into the Tri, with a default
 *  value.

 This function can be used with non-associative containers of
 std::strings to insert all of the contained strings into the
 Tri.A default value is provided with which the inserted
 string-keys will all be associated.

 \param container A non-associative container of strings.
 \param defaultValue A value_T object that will be associated with the
 inserted strings from the container.
*/
	template < template < typename, typename...> class container_T, typename... args>
	void insertFrom(const container_T<std::string, args...>& container,
			const value_T& defaultValue);
/*! Extracts values from the Tri by searching for a given string key.

  If the provided key is found, a pointer to the associated object is
  returned. If the key is not found, NULL is returned.
  This function was mainly used for testing and is not actually used in
  the rest of the program.
  \param key A string that an object is associated with in the Tri.
  \return Pointer to NULL or the associated object, if found.
*/
	const value_T* unsafeGet(const std::string& key) const;

/*! Perform beam search through the Tri using a Searcher object to
 *  find results for bad keys.

 This function provides modular beam searching functionality. It
 requires an instance of the Searcher type to implement the actual
 search logic.
 During the search, the Searcher object accumulates results according
 to its own logic. These results are repackaged and returned as a
 TolerantResult object.
 If no keys can be found, an empty vector is returned.
 The results are not sorted.
 \param searchf An instance of Searcher<derived_T>
 \return A vector of TolerantResults, which contain the found keys,
 their values and an edit distance needed to find them.
 \sa CorrectionSearcher
 \sa tolerantFind
*/
	template <typename derived_T>
	std::vector<TolerantResult> tolerantFindWith(Searcher<derived_T>& searchf) const;
/*! Error-tolerant retrieval of key/value data.

  This function attempts to use a CorrectionSearcher to find keys that
  are no more than a specified edit-distance away from a
  provided, possibly misspelled, key-string.
  If no keys can be found, an empty vector is returned.
  A call to this function is equivalent to calling
  tolerantFindWith(CorrectionSearcher(key, EditDistance))
  \param key A possibly misspelled string for which corrections will be
  searched.
  \param editDistance The maximum amount of edit-operations to perform
  during the search.
  \return A vector of TolerantResults that contain all found keys, their
  associated values and the edit-distance needed to find them;
  not sorted.
  \sa CorrectionSearcher
*/
	std::vector<TolerantResult> tolerantFind(const std::string& key, const unsigned int editDistance) const;
/*! Provides copying functionality.

  Since I disabled the implicit copy-ctor, this function can be called
  explicitly to make a deep copy of a Tri.
  A deep copy will copy all the States,Transitions and also call the
  copy ctor of all contained values.

  \return A new instance of Tri that will associated the same strings
  with the same values as the original.
*/
	Tri<value_T> makeCopy() const;

/*! Returns the number of states in the Tri

  \return Number of States currently in the Tri.
*/
	unsigned int getStates() const;
}; // class Tri

// Due to the Tri class being a template class, seperation of
// declaration and definition is unnecessary/impossible; However I
// like to do it anyway sometimes for stylistic reasons. Files that
// have the tpp.hpp suffix are meant as pseudo-translation units and
// contain the definitions of template class functions, to be included
// in their respective header files.
#include "Tri.tpp.hpp"

#endif


