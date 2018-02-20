

/*! For NoOp items, the current transChar (which is the character
 *  normally expected at the current position in the input string) is
 *  appended to the candidate string and the pointer into the input
 *  string is advanced by one.
 This is the only item that does not increase the edit operation count.
 \param transChar A character for which there is a guaranteed
 transition and which is pointed to by the current item.
 \param newStateData The information for the state about to be reached.
 \param newState The name(as number) of the state to be reached.
 \param top The current item.
 \return An item representing no-operation, other than the consumption
 of the input string. 
*/
template <typename state_T>
std::unique_ptr<item<state_T>> CorrectionSearcher::_noopItem(const unsigned char transChar, const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const {
	std::string newCandidate(top.candidate);
	newCandidate.push_back(transChar);

	// unique_ptr is created as rvalue, so this invokes the move
	// constructor rather than copy ctor(which is illegal)
	return std::unique_ptr<item<state_T>>(
		new item<state_T>(newStateData,
				  newState,
				  top.editDistance,
				  std::next(top.nextChar),
				  newCandidate));
}

/*! For substitution items, the substitutor character transChar is
 *  appended to the current candidate and the pointer into the input
 *  string is advanced, skipping the substituted character in the
 *  input string.
 \param transChar The character that is substituted into the input
 string.
 \param newStateData Information for the state that will be reached
 with the substitution.
 \param The name(as number) of the state to be reached.
 \param top The current item.
 \return An item representing a substitution of transChar into the
 current position in the input string held in the current item.
*/
template <typename state_T>
std::unique_ptr<item<state_T>> CorrectionSearcher::_substitutionItem(const unsigned char transChar, const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const {
	std::string newCandidate(top.candidate);
	newCandidate.push_back(transChar);
	return std::unique_ptr<item<state_T>>(
		new item<state_T>(newStateData,
				  newState,
				  top.editDistance + 1,
				  std::next(top.nextChar),
				  newCandidate));
}

/*! For insertion items, transChar is appended to the current
 *  candidate string, while the pointer into the input string is not advanced.
 \param transChar The character being inserted; guaranteed to have a
 transition for the current state.
 \param newStateData Information for the state reached with the
 transition.
 \param newState The name(as number) of the state to be reached.
 \param top The current item.
 \return An item representing the insertion of transChar into the input
 string at the position held in the item.
*/
template <typename state_T>
std::unique_ptr<item<state_T>> CorrectionSearcher::_insertionItem(const unsigned char transChar, const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const {
	std::string newCandidate(top.candidate);
	newCandidate.push_back(transChar);
	return std::unique_ptr<item<state_T>>(
		new item<state_T>(newStateData,
				  newState,
				  top.editDistance + 1,
				  top.nextChar,
				  newCandidate));
}

/*! For a deletion item, nothing is appended to the candidate string,
  the state is not altered and only the pointer into the input string
  is advanced by one.
  \param newStateData State information for the new item.
  \param newState The name(as number) of the state reached with
  deletion.
  \param top The current item.
  \return An item representing the deletion of one character.
*/
template <typename state_T>
std::unique_ptr<item<state_T>> CorrectionSearcher::_deletionItem(const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const {
	return std::unique_ptr<item<state_T>>(
		new item<state_T>(newStateData,
				  newState,
				  top.editDistance + 1,
				  std::next(top.nextChar),
				  top.candidate));
}


/*! For the new transposition item, thisWasPutLeft and thisWasPutRight
 *  are appended to the current candidate string to procure the new
 *  candidate. Because transposition involves two characters, the
 *  pointer into the input string is advanced by two steps for the new item.
 \param thisWasPutLeft A character of the input string that was on the
 right before the transposition.
 \param thisWasPutRight A character of the input string that was on the
 left before the transposition.
 \param newStateData The state information for the state that can be
 reached with the transposition.
 \param newState The name(as number) of the state that can be reached
 with the transposition.
 \param top The current item.
 \return A new item representing the successful transposition.
*/
template <typename state_T>
std::unique_ptr<item<state_T>> CorrectionSearcher::_transpositionItem(const unsigned char thisWasPutLeft, const unsigned char thisWasPutRight, const state_T& newStateData, const unsigned int newState, const item<state_T>& top) const {
	std::string newCandidate(top.candidate);
	newCandidate.push_back(thisWasPutLeft);
	newCandidate.push_back(thisWasPutRight);

	return std::unique_ptr<item<state_T>>(
		new item<state_T>(newStateData,
				  newState,
				  top.editDistance + 1,
				  std::next(std::next(top.nextChar)),
				  newCandidate));
}
/*! If conditions are met, pushes items for transposition, insertion
 *  and substitution onto the stack. These items are created only for
 *  one character that is guaranteed to be a transition of the current state.
 \param transChar The character of the transition of the current state
 for which items are pushed.
 \param nextState The state that is reached with the transition for
 transChar
 \param top The current item.
 \param s A reference to the stack.
 \param v A vector containing state data.
*/
template <typename state_T>
void CorrectionSearcher::_pushEditOperations(const unsigned char transChar, const unsigned int nextState, const item<state_T>& top, std::stack<std::unique_ptr<item<state_T>>>& s, const std::vector<std::unique_ptr<state_T>>& v) const {
// transposition
	// A transposition item is only pushed if there are
	// transitions for the resulting swapped characters. To check
	// this, we look ahead one char.
	auto putMeLeft(top.nextChar + 1); // putMeLeft is the char
					  // that was formerly on the right
	if(putMeLeft != _word.cend() && *putMeLeft == transChar) {
		// we have enough chars ahead to actually transpose
		// and there is a transition for the formerly right one


// putMeRight is on the left before the transposition
		const unsigned char putMeRight(*top.nextChar);  
// pretend we transpose and look ahead for the resulting state
		auto stateAfterPutMeRight(v[nextState]->transitions.find(putMeRight));
		if(stateAfterPutMeRight != v[nextState]->transitions.cend()) {
			// a transition exists; push the item
			s.push(std::move(_transpositionItem(*putMeLeft, putMeRight, *(v[stateAfterPutMeRight->second]), stateAfterPutMeRight->second, top)));
		}
	}

// push other items
	s.push(std::move(_substitutionItem(transChar, *(v[nextState]), nextState, top)));
	s.push(std::move(_insertionItem(transChar, *(v[nextState]), nextState, top)));
}


// Public member functions


/*! Returns an item with the initial state data, 0 edit distance, a
 *  pointer to the first character of the input string and an empty
 *  string as candidate for the correction. 
 \param startState The name(as number) of the initial state.
 \param v A vector of states, indices corresponding to names of states.
 \return A unique_ptr to an item, that can be used to seed a stack for searching. 
*/
template <typename state_T>
std::unique_ptr<item<state_T>> CorrectionSearcher::initialItem(const unsigned int startState, const std::vector<std::unique_ptr<state_T>>& v) const {
	return std::unique_ptr<item<state_T>>(
		new item<state_T>(*(v[startState]),
				  startState,
				  0,
				  _word.cbegin(),
				  ""));
}

/*!
  Most of the searching work is done in this function. It is meant to be
  called in a loop, in which the caller is maintaining a
  stack. _feedStack does not maintain a stack itself, it merely examines
  the provided item to decide what to push on the stack. The stack
  itself is not examined or modified in any way, other than pushing
  items onto it. The number of items pushed is a function of the
  provided transitions and the top item. This function must be called for the
  CorrectionSearcher to accumulate any results; the saving of results is
  a side effect of this function.

  \todo Destructive update
  This function would be much neater if it returned a list/vector of
  produced items instead of mutating the stack state. However, this would create need for additional
  boxing/unboxing, which seems wasteful :(
  \param s A stack of unique_ptrs to items; this will be updated
  destructively.
  \param v A vector that provides a mapping from state numbers to state
  data.
  \param top The item that will be examined; supposed to be the already
  removed top of the given stack. 
  \sa _pushEditOperations
*/
template <typename state_T>
void CorrectionSearcher::feedStack(std::stack<std::unique_ptr<item<state_T>>>& s, const std::vector<std::unique_ptr<state_T> >& v, const item<state_T>& top) {
// Successful candidate?
	if(top.nextChar == _word.cend() && top.stateData.endState) {
		// item represents end state and we have reached end
		// of input string
		auto i(_results.find(top.candidate));
		if(i == _results.cend() || i->second.second > top.editDistance) {
			// if the same candidate was already found, we
			// only replace it if we have better edit distance
			_results.insert(std::make_pair(top.candidate, std::make_pair(top.state, top.editDistance)));
		}
	} // if success

// perhaps the word is not misspelled? do a NoOp!
	if(top.nextChar != _word.cend()) {
		// we are not at the end of input
		auto foundState(top.stateData.transitions.find(*top.nextChar));
		if(foundState != top.stateData.transitions.end()) {
			s.push(std::move(_noopItem(*top.nextChar,
						   *(v[foundState->second]),
						   foundState->second,
						   top)));
		} // if foundState
	} // if _word.cend()

// edit operations
	if(top.editDistance < _cutoffDistance) {
		// never produce items that are more edit ops than the cutoff
		if(top.nextChar != _word.cend()) { // not at end of input
// we can only delete what is in the input string, so this happens
// outside of the loop below
			s.push(std::move(_deletionItem(top.stateData,
						       top.state,
						       top)));
		} // nextChar != cend
// for all transitions in this state
		for(auto t(top.stateData.transitions.cbegin()); t != top.stateData.transitions.cend(); ++t) {
// other edit operations, happening for every char for which there is
// a transition
			_pushEditOperations(t->first,
					    t->second,
					    top,
					    s,
					    v);
		} // for
	}  // if editDistance
} // feedStack

const result_type& CorrectionSearcher::getResults() const {
	return _results;
}

