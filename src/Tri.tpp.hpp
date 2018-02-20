

#include <iostream>


template <class value_T>
Tri<value_T>::Tri()
	: _v(), _values() {
// create new state and immediately put it in the state vector.
	_v.push_back(std::unique_ptr<State>(new State));
}

template <class value_T>
typename Tri<value_T>::state_T Tri<value_T>::_newStatef() {
	unsigned int n(getStates());
	if(n == (unsigned int)~0) {
		// sorry we cannot deal with more states... panic
		throw std::runtime_error("error in Tri::_newStatef: Too many states.");
	}
// push new state in state vector
	_v.push_back(std::unique_ptr<State>(new State));
// the name of the latest state is always the size of the vector minus
// one - but we got the size before we pushed an element so its + 1 again
	return n;
} // Tri ctor

template <class value_T>
void Tri<value_T>::insert(const std::string key, const value_T& newValue) {
	state_T currentState(0);
	transitions_T* currentTransitions;
// iterate over input string characters
	for(auto c(key.cbegin()); c != key.cend(); ++c) {
		currentTransitions = &(_v[currentState]->transitions);
		auto iter = currentTransitions->find(*c);
		if(iter != currentTransitions->end()) {
// there is a transition from the current state with the current
// character to another state
			currentState = iter->second;
		} else {
// no transition found, create a new state and transition to it
			(*currentTransitions)[*c] = currentState = _newStatef();
		}
	} // for
// set the state we are left in to be an end state
	_v[currentState]->endState = true;
// associate the name(number) of the current state with the value to
// be inserted
	_values[currentState] = newValue;
} // insert

template <typename value_T>
void Tri<value_T>::insert(const std::pair<const std::string, const value_T>& p) {
	insert(p.first, p.second);
} // insert

template <typename value_T>
template < template < typename, typename...> class container_T, typename... args>
void Tri<value_T>::insertFrom(const container_T<std::string, args...>& v, const value_T& defaultValue) {
	for(auto iter = v.begin();iter != v.end(); iter++) {
		insert(*iter, defaultValue);
	}
}


template <class value_T>
const value_T* Tri<value_T>::unsafeGet(const std::string& key) const {
	const transitions_T* currentTransitions;
	state_T currentState(0);
// there is some commonality between get and insert, this could be refactored.
	for(auto c(key.cbegin()); c != key.cend(); ++c) {
		currentTransitions = &(_v[currentState]->transitions);
		auto iter = currentTransitions->find(*c);
		if(iter != currentTransitions->cend()) {
// there is a transition from the current state with the current
// character to another state
			currentState = iter->second;
		} else {
			// no transition found, the key cannot be in
			// the Tri, return NULL pointer
			return NULL;
		}
	} // for
	if(_v[currentState]->endState) {
// entire input String consumed - and the resulting state is an end
// state - return the value!
		return &(_values.find(currentState)->second);
	} else {
		// the input string was consumed, but the state we
		// ended up in happens not to be an end state. Sorry,
		// no dice!
		return NULL;
	}
} // unsafe_get
	
template <class value_T>
unsigned int Tri<value_T>::getStates() const {
	return _v.size();
} // getStates

template <typename value_T>
template <typename derived_T>
std::vector<typename Tri<value_T>::TolerantResult> Tri<value_T>::tolerantFindWith(Searcher<derived_T>& searchf) const {

// the stack is maintained here, the searcher does not do anything
// except push things onto it
/// \todo stack is using unique_ptr because we have to pass items
// around. This is nice and safe, but probably a bad idea.
// the Searcher exports an item type, which is the type of things we
// store on the stack
	std::stack< std::unique_ptr<typename Searcher<derived_T>::template item<State>::type>> s;
// Searcher also provides a seed item to start with
	for(s.push(std::move(searchf.initialItem(0, _v))); !s.empty();) {
// now we just do a depth-first search of a graph, hoping that it will terminate
		std::unique_ptr<typename Searcher<derived_T>::template item<State>::type> top = std::move(s.top());
		s.pop();
// this will put new items onto the stack
		searchf.feedStack(s, _v, *top);
	}
	// extract values for states and package up results
	const typename Searcher<derived_T>::result_type intermediateResults(searchf.getResults());
	std::vector<Tri<value_T>::TolerantResult> finalResults;
// the Searcher does not need access to the Tris internal associated
// values, so it only returns the state numbers of found keys, which
// we use to extract the real values.
	for(auto kv(intermediateResults.cbegin()); kv != intermediateResults.cend(); ++kv) {
		finalResults.push_back(TolerantResult(kv->first, _values.at(kv->second.first), kv->second.second));
	}
	return std::move(finalResults);
} // tolerantFindWith

template <typename value_T>
Tri<value_T> Tri<value_T>::makeCopy() const {
	Tri<value_T> newTri;
// careful, tri always has startstate 0 on creation
	newTri._v[0] = std::unique_ptr<State>(&((new State)->copyFrom(*_v[0])));
// deep copy all the states, skip the first one
	for(auto i(std::next(_v.cbegin())); i != _v.cend(); ++i) {
		newTri._v.push_back(std::unique_ptr<State>((&(new State)->copyFrom(**i))));
	}

// this will call the copy assignment operator of unordered_map
	newTri._values = _values;
	return std::move(newTri);
} // makeCopy

template <typename value_T>
std::vector<typename Tri<value_T>::TolerantResult> Tri<value_T>::tolerantFind(const std::string& key, const unsigned int editDistance) const {
	std::unique_ptr<CorrectionSearcher> searchf(new CorrectionSearcher(key, editDistance));
	return tolerantFindWith(*searchf);
} // tolerantFind
	
