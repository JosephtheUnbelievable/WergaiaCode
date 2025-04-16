#include <bits/stdc++.h>
using namespace std;

// Structure representing a syllable with optional stress and parentheses
struct Syllable{
	char weight; // 'L' = light, 'H' = heavy
	bool hasLeftParenthesis = false, hasRightParenthesis = false;
	bool hasStress = false;
};

// Helper function to mark violations in a range
void assignViolation(int leftBound, int rightBound, vector<int> &violation){
	for(int i = leftBound;i<=rightBound;i++){
		violation[i] = 1;
	}
}

// Converts a syllable word structure to a readable string with stress and parentheses
string printInfo(vector<Syllable> word){
	int wordLen = word.size();
	string output = "";
	for(int i = 0;i<wordLen;i++){
		if(word[i].hasLeftParenthesis) output+='(';
		if(word[i].hasStress) output+='\'';
		output+=word[i].weight;
		if(word[i].hasRightParenthesis) output+=')';
	}
	return output;
}

// Finds the locations of parenthesis pairs in the word
vector<pair<int,int>> findLocation(vector<Syllable> word){
	vector<pair<int,int>> parensLocation;
	for(int i = 0;i<word.size();i++){
		Syllable syllab = word[i]; 
		if(syllab.hasLeftParenthesis){
			parensLocation.push_back(make_pair(i,-1)); 
		}
		if(syllab.hasRightParenthesis){
			parensLocation.back().second = i;
		}
	}
	return parensLocation;
}

// Trochee constraint: stressed syllable should be on the left in two-syllable feet
vector<int> Trochee(vector<Syllable> word){
	int wordLen = word.size();
	vector<int> violation(wordLen,0); 
	
	vector<pair<int,int>> parensLocation = findLocation(word);
	
	for(auto parens: parensLocation){
		int leftParens = parens.first, rightParens = parens.second;
		int len = rightParens - leftParens+1;
		
		if(len >= 3){ // punish feet with 3+ syllables
			assignViolation(leftParens, rightParens, violation);
		}
		if(len == 2){ // check stress alignment
			if(word[rightParens].hasStress or !word[leftParens].hasStress){
				assignViolation(leftParens, rightParens, violation);
			}
		}
		if(len == 1){ // penalize single light syllable
			if(word[leftParens].weight == 'L'){
				assignViolation(leftParens,rightParens,violation);
			}
		}
	}
	return violation;
}

// Iamb constraint: stress should be on the right in two-syllable feet
vector<int> Iamb(vector<Syllable> word){
	int wordLen = word.size();
	vector<int> violation(wordLen,0); 
	
	vector<pair<int,int>> parensLocation = findLocation(word);
	
	for(auto parens: parensLocation){
		int leftParens = parens.first, rightParens = parens.second;
		int len = rightParens - leftParens+1;
		
		if(len >= 3){ // punish feet with 3+ syllables
			assignViolation(leftParens, rightParens, violation);
		}
		if(len == 2){ // check stress alignment
			if(!word[rightParens].hasStress or word[leftParens].hasStress){
				assignViolation(leftParens, rightParens, violation);
			}
		}
		if(len == 1){ // penalize single light syllable
			if(word[leftParens].weight == 'L'){
				assignViolation(leftParens,rightParens,violation);
			}
		}
	}
	return violation;
}

// ParseLeft: penalizes any syllable that is not inside a parenthesis
vector<int> ParseLeft(vector<Syllable> word){
	int wordLen = word.size();
	vector<int> violation(wordLen,0); 
	
	vector<pair<int,int>> parensLocation = findLocation(word);
	for(int i = 0;i<wordLen;i++){
		if(!word[i].hasLeftParenthesis && !word[i].hasRightParenthesis){
			violation[i] = 1;
		}
	}
	return violation;
}

// ParseRight: same as ParseLeft, but reverses the resulting violation vector
vector<int> ParseRight(vector<Syllable> word){
	int wordLen = word.size();
	vector<int> violation(wordLen,0); 
	
	vector<pair<int,int>> parensLocation = findLocation(word);
	for(int i = 0;i<wordLen;i++){
		if(!word[i].hasLeftParenthesis && !word[i].hasRightParenthesis){
			violation[i] = 1;
		}
	}
	
	reverse(violation.begin(),violation.end());
	return violation;
}

// Converts an input string into a vector of Syllables, handling stress (') and weights (L/H)
vector<Syllable> parseString(string inputSequence){
	int cnt = 0;
	for(auto c: inputSequence){
		if(c!='\'') cnt++;
	}
	cout<<cnt<<endl;
	vector<Syllable> word(cnt);
	int index = 0;
	for(int i = 0;i<inputSequence.size();i++){
		if(inputSequence[i] == '\'') word[index].hasStress = true;
		else word[index].weight = inputSequence[i], index++;
	}
	return word;
}

// List of constraints to be applied in SerialOT
vector<function<vector<int>(vector<Syllable>)>> constraints = {
    Trochee,
    ParseLeft,
    Iamb,
    ParseRight
};

// Main function implementing Serial Optimality Theory
vector<Syllable> SerialOT(vector<Syllable> word,
const vector<function<vector<int>(vector<Syllable>)>>& constraints){
	int wordLen = word.size();
	vector<pair<vector<Syllable>, vector<int>>> candidates;
 	
	// Converts a binary violation vector to an integer for easy comparison
	auto score = [](vector<int> violation) {
	    int val = 0;
	    for (int i = 0; i < violation.size(); i++) {
	        val *= 2;
	        val += violation[i];
	    }
	    return val;	
	};
	
	// Try adding a foot (pair of parentheses) of length 1 or 2 and stress in all combinations
	for(int leftParens = 0; leftParens < wordLen; leftParens++){
		for(int rightParens = leftParens; rightParens < wordLen; rightParens++){
			if(rightParens-leftParens+1 > 2) continue;
			
			vector<Syllable> copy = word;
			// Skip if parentheses already present at either bound
			if(copy[leftParens].hasLeftParenthesis == true or
			copy[leftParens].hasRightParenthesis == true or
			copy[rightParens].hasLeftParenthesis == true or
			copy[rightParens].hasRightParenthesis == true) continue;
			
			// Apply parentheses to the selected span
			copy[leftParens].hasLeftParenthesis = true;
			copy[rightParens].hasRightParenthesis = true;
			
			int footSize = rightParens - leftParens + 1;
	        // Try all combinations of stress for this foot
	        for (int mask = 0; mask < (1 << footSize); ++mask) {
	        	//you can't have a foot that has no stress
	        	if(mask == 0) continue;
	        	
	            vector<Syllable> stressedCopy = copy;
	            for (int i = 0; i < footSize; ++i) {
	                if (mask & (1 << i)) {
	                    stressedCopy[leftParens + i].hasStress = true;
	                } else {
	                    stressedCopy[leftParens + i].hasStress = false;
	                }
	            }
	            
	            // Evaluate constraint violations for this candidate
	            vector<int> candidateScore;
	            for (const auto& constraint : constraints) {
	                vector<int> violation = constraint(stressedCopy);
	                candidateScore.push_back(score(violation));
	            }
	            
	            // Store candidate
	            candidates.push_back(make_pair(stressedCopy, candidateScore));
	        }	
		}
	}
	
	// Also add the original (unchanged) word to candidates
	vector<int> candidateScore;
	for (const auto& constraint : constraints) {
	    vector<int> violation = constraint(word);
	    candidateScore.push_back(score(violation));
	}
	candidates.push_back(make_pair(word,candidateScore));

	// Sort candidates by lexicographically smallest violation vector
	sort(candidates.begin(), candidates.end(), 
	[](const pair<vector<Syllable>, vector<int>>& a, const pair<vector<Syllable>, vector<int>>& b) {
    	return a.second < b.second;
	});
	
	// Print all candidates and their scores

	int optionNum = 1;
	for(const auto& candidate : candidates){
	    cout << "Option " << optionNum++ << ": " << printInfo(candidate.first) << " | Scores: ";
	    for (int i = 0; i < candidate.second.size(); ++i) {
	        string constraintName;
	        if (i == 0) constraintName = "Trochee";
	        else if (i == 1) constraintName = "ParseLeft";
	        else if (i == 2) constraintName = "Iamb";
	        else if (i == 3) constraintName = "ParseRight";
	        cout << constraintName << "=" << candidate.second[i];
	        if (i != candidate.second.size() - 1) cout << ", ";
	    }
	    cout << "\n";
	}

	cout << "✅ Selected Best Candidate: " << printInfo(candidates[0].first) << "\n";
	cout << "====================================\n";
	
	
	
	
	
	
	
	return candidates[0].first; // Return best candidate
}

int main() {
	string inputSequence; cin>>inputSequence;
	vector<Syllable> v = parseString(inputSequence);

	// Repeat SerialOT until no improvement occurs
	while(true){
		vector<Syllable> ret = SerialOT(v,constraints);
		if(printInfo(ret) == printInfo(v)) break; // Stop if no change
		else{
			 v = ret; // Update word
		}
	}
	
	return 0;
}