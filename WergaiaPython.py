from typing import List, Tuple, Callable
from copy import deepcopy

# Structure representing a syllable with optional stress and parentheses
class Syllable:
    def __init__(self):
        self.weight = ''  # 'L' = light, 'H' = heavy
        self.hasLeftParenthesis = False
        self.hasRightParenthesis = False
        self.hasStress = False

# Helper function to mark violations in a range
def assignViolation(leftBound: int, rightBound: int, violation: List[int]):
    for i in range(leftBound, rightBound + 1):
        violation[i] = 1

# Converts a syllable word structure to a readable string with stress and parentheses
def printInfo(word: List[Syllable]) -> str:
    output = ""
    for syllab in word:
        if syllab.hasLeftParenthesis:
            output += '('
        if syllab.hasStress:
            output += '\''
        output += syllab.weight
        if syllab.hasRightParenthesis:
            output += ')'
    return output

# Finds the locations of parenthesis pairs in the word
def findLocation(word: List[Syllable]) -> List[Tuple[int, int]]:
    parensLocation = []
    for i, syllab in enumerate(word):
        if syllab.hasLeftParenthesis:
            parensLocation.append((i, -1))
        if syllab.hasRightParenthesis:
            parensLocation[-1] = (parensLocation[-1][0], i)
    return parensLocation

# Trochee constraint: stressed syllable should be on the left in two-syllable feet
def Trochee(word: List[Syllable]) -> List[int]:
    wordLen = len(word)
    violation = [0] * wordLen
    parensLocation = findLocation(word)

    for parens in parensLocation:
        leftParens, rightParens = parens
        length = rightParens - leftParens + 1

        if length >= 3:
            assignViolation(leftParens, rightParens, violation)
        if length == 2:
            if word[rightParens].hasStress or not word[leftParens].hasStress:
                assignViolation(leftParens, rightParens, violation)
        if length == 1:
            if word[leftParens].weight == 'L':
                assignViolation(leftParens, rightParens, violation)
    return violation

# Iamb constraint: stress should be on the right in two-syllable feet
def Iamb(word: List[Syllable]) -> List[int]:
    wordLen = len(word)
    violation = [0] * wordLen
    parensLocation = findLocation(word)

    for parens in parensLocation:
        leftParens, rightParens = parens
        length = rightParens - leftParens + 1

        if length >= 3:
            assignViolation(leftParens, rightParens, violation)
        if length == 2:
            if not word[rightParens].hasStress or word[leftParens].hasStress:
                assignViolation(leftParens, rightParens, violation)
        if length == 1:
            if word[leftParens].weight == 'L':
                assignViolation(leftParens, rightParens, violation)
    return violation

# ParseLeft: penalizes any syllable that is not inside a parenthesis
def ParseLeft(word: List[Syllable]) -> List[int]:
    wordLen = len(word)
    violation = [0] * wordLen
    for i in range(wordLen):
        if not word[i].hasLeftParenthesis and not word[i].hasRightParenthesis:
            violation[i] = 1
    return violation

# ParseRight: same as ParseLeft, but reverses the resulting violation vector
def ParseRight(word: List[Syllable]) -> List[int]:
    wordLen = len(word)
    violation = [0] * wordLen
    for i in range(wordLen):
        if not word[i].hasLeftParenthesis and not word[i].hasRightParenthesis:
            violation[i] = 1
    violation.reverse()
    return violation

# Converts an input string into a vector of Syllables, handling stress (') and weights (L/H)
def parseString(inputSequence: str) -> List[Syllable]:
    cnt = sum(1 for c in inputSequence if c != '\'')
    print(cnt)
    word = [Syllable() for _ in range(cnt)]
    index = 0
    for c in inputSequence:
        if c == '\'':
            word[index].hasStress = True
        else:
            word[index].weight = c
            index += 1
    return word

# List of constraints to be applied in SerialOT
constraints: List[Callable[[List[Syllable]], List[int]]] = [
    Trochee,
    ParseLeft,
    Iamb,
    ParseRight
]

# Main function implementing Serial Optimality Theory
def SerialOT(word: List[Syllable],
             constraints: List[Callable[[List[Syllable]], List[int]]]) -> List[Syllable]:
    wordLen = len(word)
    candidates = []

    # Converts a binary violation vector to an integer for easy comparison
    def score(violation: List[int]) -> int:
        val = 0
        for v in violation:
            val = val * 2 + v
        return val

    for leftParens in range(wordLen):
        for rightParens in range(leftParens, wordLen):
            if rightParens - leftParens + 1 > 2:
                continue

            copy = deepcopy(word)
            if (copy[leftParens].hasLeftParenthesis or copy[leftParens].hasRightParenthesis or
                copy[rightParens].hasLeftParenthesis or copy[rightParens].hasRightParenthesis):
                continue

            copy[leftParens].hasLeftParenthesis = True
            copy[rightParens].hasRightParenthesis = True

            footSize = rightParens - leftParens + 1
            for mask in range(1, 1 << footSize):  # skip 0 (no stress)
                stressedCopy = deepcopy(copy)
                for i in range(footSize):
                    stressedCopy[leftParens + i].hasStress = (mask & (1 << i)) != 0

                candidateScore = []
                for constraint in constraints:
                    violation = constraint(stressedCopy)
                    candidateScore.append(score(violation))

                candidates.append((stressedCopy, candidateScore))

    # Add original unchanged word
    candidateScore = []
    for constraint in constraints:
        violation = constraint(word)
        candidateScore.append(score(violation))
    candidates.append((word, candidateScore))

    # Sort by lexicographically smallest violation vector
    candidates.sort(key=lambda x: x[1])

    # Print all candidates and their scores
    optionNum = 1
    for candidate in candidates:
        print(f"Option {optionNum}: {printInfo(candidate[0])} | Scores: ", end="")
        for i, scoreVal in enumerate(candidate[1]):
            constraintName = ["Trochee", "ParseLeft", "Iamb", "ParseRight"][i]
            print(f"{constraintName}={scoreVal}", end=", " if i != len(candidate[1]) - 1 else "")
        print()
        optionNum += 1

    print(f"✅ Selected Best Candidate: {printInfo(candidates[0][0])}")
    print("====================================")

    return candidates[0][0]  # Return best candidate

# Main runner
if __name__ == "__main__":
    inputSequence = input()
    v = parseString(inputSequence)

    # Repeat SerialOT until no improvement occurs
    while True:
        ret = SerialOT(v, constraints)
        if printInfo(ret) == printInfo(v):
            break
        else:
            v = ret
