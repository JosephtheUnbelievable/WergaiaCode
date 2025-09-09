
# Serial Optimality Theory — README

This repository documents a simplified **Serial Optimality Theory (OT)** evaluator for metrical phonology implemented in **C++** and **Python**.

It searches over candidate feet (parentheses over spans of length 1–2) and stress placements, evaluates them against a fixed constraint set, and repeatedly chooses the lexicographically best candidate until no further improvement occurs.

---

## Overview

- **Input:** a string of syllable weights with optional stress markers  
  - `L` = light syllable  
  - `H` = heavy syllable  
  - `'` (apostrophe) = places stress on the **following** syllable (e.g., ` 'L ` means a stressed light syllable)
- **Output:** the optimal candidate after serial application of constraints, with a printed candidate table per step.

---

## Constraints (informal)

1. **Trochee**  
   Two-syllable foot prefers **left** stress.  
   Violations:
   - any foot with **3+ syllables**
   - wrong stress alignment in a 2-syllable foot (right stressed or left unstressed)
   - single **light** (`L`) syllable foot

2. **Iamb**  
   Two-syllable foot prefers **right** stress.  
   Violations:
   - any foot with **3+ syllables**
   - wrong stress alignment in a 2-syllable foot (left stressed or right unstressed)
   - single **light** (`L`) syllable foot

3. **ParseLeft**  
   Penalizes syllables **not parsed** into feet (not inside parentheses).

4. **ParseRight**  
   Same as `ParseLeft`, but the violation vector is **reversed** (right-edge preference).

> Each constraint returns a binary violation vector across positions. We convert that vector to an integer (treating it as a binary number). Candidates are ranked **lexicographically** by the tuple of per-constraint integers.

---

## Algorithm (Serial OT)

1. From the current form, generate candidates by:
   - adding a foot over spans of length **1–2** that are not already bounded by parentheses;
   - assigning stress to any non-empty subset of positions in the new foot (exclude the “no stress” case);
   - also include the **unchanged** input as a candidate.
2. Evaluate every candidate against the constraint list to get a vector of integer scores.
3. Pick the **lexicographically smallest** score vector → the winner.
4. Replace the current form with the winner and repeat **until stable** (no further improvement).

---

## Build & Run

### C++
```bash
g++ -std=c++17 -O2 -o serial_ot Wergaiac++.cpp
./serial_ot
```
Then type an input string like `HLLH` or `'LHL` and press Enter.

### Python
```bash
python Wergaiapython.py
```
Then type an input string and press Enter.

---

## Example Input / Output

### Sample Input 1
```
HLLH
```

### Sample Output 1 (three serial steps, illustrative)
```
4
Option 1: ('HL)LH | Scores: Trochee=0, ParseLeft=3, Iamb=12, ParseRight=12
Option 2: ('H)LLH | Scores: Trochee=0, ParseLeft=7, Iamb=0, ParseRight=14
Option 3: H('LL)H | Scores: Trochee=0, ParseLeft=9, Iamb=6, ParseRight=9
Option 4: HL('LH) | Scores: Trochee=0, ParseLeft=12, Iamb=3, ParseRight=3
Option 5: HLL('H) | Scores: Trochee=0, ParseLeft=14, Iamb=0, ParseRight=7
Option 6: HLLH | Scores: Trochee=0, ParseLeft=15, Iamb=0, ParseRight=15
Option 7: HL('L)H | Scores: Trochee=2, ParseLeft=13, Iamb=2, ParseRight=11
Option 8: HL(L'H) | Scores: Trochee=3, ParseLeft=12, Iamb=0, ParseRight=3
Option 9: HL('L'H) | Scores: Trochee=3, ParseLeft=12, Iamb=3, ParseRight=3
Option 10: H('L)LH | Scores: Trochee=4, ParseLeft=11, Iamb=4, ParseRight=13
Option 11: H(L'L)H | Scores: Trochee=6, ParseLeft=9, Iamb=0, ParseRight=9
Option 12: H('L'L)H | Scores: Trochee=6, ParseLeft=9, Iamb=6, ParseRight=9
Option 13: (H'L)LH | Scores: Trochee=12, ParseLeft=3, Iamb=0, ParseRight=12
Option 14: ('H'L)LH | Scores: Trochee=12, ParseLeft=3, Iamb=12, ParseRight=12
✅ Selected Best Candidate: ('HL)LH
====================================
Option 1: ('HL)('LH) | Scores: Trochee=0, ParseLeft=0, Iamb=15, ParseRight=0
Option 2: ('HL)L('H) | Scores: Trochee=0, ParseLeft=2, Iamb=12, ParseRight=4
Option 3: ('HL)LH | Scores: Trochee=0, ParseLeft=3, Iamb=12, ParseRight=12
Option 4: ('HL)('L)H | Scores: Trochee=2, ParseLeft=1, Iamb=14, ParseRight=8
Option 5: ('HL)(L'H) | Scores: Trochee=3, ParseLeft=0, Iamb=12, ParseRight=0
Option 6: ('HL)('L'H) | Scores: Trochee=3, ParseLeft=0, Iamb=15, ParseRight=0
✅ Selected Best Candidate: ('HL)('LH)
====================================
Option 1: ('HL)('LH) | Scores: Trochee=0, ParseLeft=0, Iamb=15, ParseRight=0
✅ Selected Best Candidate: ('HL)('LH)
====================================
```

Notes:
- The stray leading integer sometimes seen in older logs (e.g., `4`) was the syllable count printed by an earlier `parseString`; the current README examples omit that.
- Exact numeric scores may vary if you change constraint order or implementation details. The shape and formatting should match this template in your run.

---

## Repository Layout
```
.
├── ExampleInputOutput/         # (optional) extra examples
├── Wergaiac++.cpp              # C++ implementation
├── Wergaiapython.py            # Python implementation
└── README.md                   # This file
```

## License
Choose a license (e.g., MIT) and add a LICENSE file if you plan to share code publicly.
