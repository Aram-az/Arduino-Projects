/* -------------------------------------------------------------
   Adaptive Markov ε-Greedy RPS strategy
   – minimal globals + makeRPSChoice()
   – no other framework code modified
   ------------------------------------------------------------- */

/* ---------- strategy state ---------- */
int trans[3][3] = {{0}};   // transition counts: prev→next
int epsPercent  = 15;      // exploration chance (5–25 %)
int noGain      = 0;       // rounds since last win

/* ---------- core decision function ---------- */
RPSChoice makeRPSChoice()
{
  /* 1 · first round: play random */
  if (roundsPlayed == 0)
      return randRPS();

  /* 2 · update transition table when two moves exist */
  if (roundsPlayed > 1) {
      int p = opponentChoices[roundsPlayed - 2];   // prev-prev move
      int q = opponentChoices[roundsPlayed - 1];   // last move
      if (p >= 0 && p < 3 && q >= 0 && q < 3)      // guard range
          trans[p][q]++;
  }

  /* 3 · pull last-round info */
  int        oppPrev = opponentChoices[roundsPlayed - 1];
  if (oppPrev < 0)                                 // safety guard
      return randRPS();
  RoundResult res = roundResults[roundsPlayed - 1];

  /* 4 · predict opponent’s next move */
  int *row = trans[oppPrev];
  RPSChoice pred;
  if      (row[0] >= row[1] && row[0] >= row[2]) pred = RPS_Rock;
  else if (row[1] >= row[2])                     pred = RPS_Paper;
  else                                           pred = RPS_Scissors;

  /* 5 · exploit prediction */
  RPSChoice choice = winningChoiceAgainst(pred);

  /* 6 · ε-greedy exploration */
  if (random(100) < epsPercent)
      choice = randRPS();

  /* 7 · adapt ε and track cold streak */
  if (res == Round_Won) {
      noGain = 0;
      if (epsPercent > 5)  epsPercent -= 2;        // shrink ε
  } else {
      noGain++;
      if (epsPercent < 25) epsPercent += 2;        // grow ε
  }

  /* 8 · auto-reset if six rounds without a win */
  if (noGain >= 6) {
      memset(trans, 0, sizeof(trans));
      epsPercent = 15;
      noGain     = 0;
  }

  return choice;
}
