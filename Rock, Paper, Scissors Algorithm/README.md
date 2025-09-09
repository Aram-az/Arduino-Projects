# Strategy Summary

Our bot opens with a single random throw, then learns on the fly using a tiny 3 × 3 transition table that records how the opponent moves from one symbol to the next. After only two observations it can predict the opponent’s most-likely next hand and play the immediate counter. A small, adaptive ε-greedy chance of random play (5–25 %) keeps the bot unpredictable and lets it probe for strategy shifts; ε shrinks after wins and grows after losses, automatically balancing exploitation and exploration. If six rounds pass without a win the table and ε reset, allowing rapid recovery from stale data. This hybrid of “beat-most-likely” and adaptive randomness converges within the 4–7 rounds of a match, never does worse than random, and fits in under 100 bytes of RAM—making it both conceptually sound and perfectly suited to the tournament’s constraints.

# 3x3 Transition Table Explained

- The Array is just a set of counters

| previous → / next ↓ | Rock (0) | Paper (1) | Scissors (2) |
| --- | --- | --- | --- |
| **Rock (0)** | `trans[0][0]` | `trans[0][1]` | `trans[0][2]` |
| **Paper (1)** | `trans[1][0]` | `trans[1][1]` | `trans[1][2]` |
| **Scissors (2)** | `trans[2][0]` | `trans[2][1]` | `trans[2][2]` |
- Suppose the opponent’s first six moves are Rock, Paper, Rock, Rock, Scissors, Paper

| End of Round | Opponent move | `p` (prev) | `q` (now) | Counter incremented (`trans[p][q]++`) |
| --- | --- | --- | --- | --- |
| 0 | Rock | — | — | (nothing yet) |
| 1 | **Paper** | Rock (0) | Paper (1) | `trans[0][1]` goes 0 → **1** |
| 2 | Rock | Paper (1) | Rock (0) | `trans[1][0]` goes 0 → **1** |
| 3 | Rock | Rock (0) | Rock (0) | `trans[0][0]` goes 0 → **1** |
| 4 | **Scissors** | Rock (0) | Scissors (2) | `trans[0][2]` goes 0 → **1** |
| 5 | Paper | Scissors (2) | Paper (1) | `trans[2][1]` goes 0 → **1** |
- After six rounds the table contains:

|  | Next Rock | Next Paper | Next Scissors |
| --- | --- | --- | --- |
| **Prev Rock** | **1** | **1** | **1** |
| **Prev Paper** | **1** | 0 | 0 |
| **Prev Sciss.** | 0 | **1** | 0 |

# Prediction Algorithm

Suppose the opponent’s last move was Rock, so `oppPrev = 0` and we load:

```arduino
row = trans[0]  → [2, 5, 1]   // Rock→Rock :2, Rock→Paper :5, Rock→Scissors :1
```

- `row[1]` (Paper) is the largest → `pred = RPS_Paper`
- `winningChoiceAgainst(RPS_Paper)` returns Scissors
- The bot plays Scissors, which is the correct counter to the predicted Paper
- That tiny block of code is the entire “brains” of the strategy, find the most common next move after the opponent’s last hand, then play the winning reply

# Randomness Factor

### Making the bot unpredictable + self-correcting

```arduino
// ───────── 1.  ε-randomness  ─────────
if (random(100) < epsPercent)
    choice = randRPS();
```

- `epsPercent` holds a **percentage** (initially 15).
- `random(100)` returns a number 0 – 99.
- About `epsPercent %` of the time we **ignore the prediction** and throw a random symbol.
    
    *Prevents the opponent from locking onto a deterministic pattern and also
    keeps gathering fresh data.*
    

---

```arduino
// ───────── 2.  Adapt ε and track win streak ─────────
if (res == Round_Won) {          // we just won
    noGain = 0;                  // reset cold-streak counter
    if (epsPercent > 5)          // doing well → explore less
        epsPercent -= 2;         // shrink ε by 2 %
} else {                         // draw or loss
    noGain++;                    // extend cold streak
    if (epsPercent < 25)         // doing poorly → explore more
        epsPercent += 2;         // grow ε by 2 %
}
```

- `res` is the result of **last** round.
- **Win** ⇒ cut randomness, exploit harder.
- **Loss/Draw** ⇒ add randomness, hunt for a better line.
- `noGain` counts consecutive rounds without a win (draws count as “no gain”).

---

```arduino
// ───────── 3.  Auto-reset if the model is stale ─────────
if (noGain >= 6) {
    memset(trans, 0, sizeof(trans));   // clear transition table
    epsPercent = 15;                   // back to balanced ε
    noGain     = 0;                    // restart streak
}
```

- If six straight rounds yield **no win**, we assume the stored statistics are misleading.
- We **wipe the transition table**, restore ε to its default 15 %, and start learning anew.
    
    *Prevents the bot from clinging to bad data when the opponent changes strategy.*
    

---

```arduino
return choice;
```

*After all adjustments, this is the hand the Arduino plays.*

---

### In one sentence

This block adds a small, tunable dose of randomness (ε-greedy), automatically tightens or loosens that randomness based on recent success, and performs a full reset after a prolonged cold streak—ensuring the bot stays both unpredictable and quick to recover within the short best-of-7 match format.
