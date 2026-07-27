#!/usr/bin/env python3

from model import Word, Clue, State
from numba import njit
import heapq

TOP_K_CANDIDATES = 5

def load_words(path: str) -> set[Word]:
    ret: set[Word] = set()
    with open(path, "r") as f:
        lines = f.readlines()
        ret = {Word.from_eng_str(it.strip()) for it in lines}
    return ret

def get_valid(state: State, words: set[Word]) -> set[Word]:
    return {it for it in words if state.check(it)}

@njit
def pick_words(
    state: State,
    answer_candidates: set[Word],
    words: set[Word],
    *,
    top_k: int = 1
) -> list[tuple[float, Word]]:

    ret: list[tuple[float, Word | None]] = [(len(answer_candidates), i) for i in range(top_k)]
    heapq.heapify(ret)

    for i, it in enumerate(words):
        print(f"Checking {it}")
        total_valid_count = 0
        clues: dict[Clue, int] = {} # Clue, weight

        # Group clues
        for jt in answer_candidates:
            cur_clue = Clue.from_words(it, jt)
            if not clues.get(cur_clue):
                clues[cur_clue] = 0
            clues[cur_clue] += 1

        for clue, weight in clues.items():
            cur_valid_count = 0
            new_state = state & State.from_clue(clue)
            for word in answer_candidates:
                if new_state.check(word):
                    cur_valid_count += 1
            total_valid_count += cur_valid_count * weight
        heapq.heappushpop(ret, (total_valid_count / len(answer_candidates), it))

    return ret

def main() -> None:
    print("Loading word list...")
    words = load_words("./data/word_list.txt")
    answer_candidates = words.copy()
    state = State() # TODO: length?

    while True:
        print(f"Top {TOP_K_CANDIDATES} guessing candidates:")
        suggestions = sorted(pick_words(state, answer_candidates, words, top_k=TOP_K_CANDIDATES))
        print(suggestions)

        while True:
            try:
                guess, clue = input("Guess state?").split()
                break
            except:
                print("Wrong input!")
        state &= State.from_clue(Clue(Word.from_eng_str(guess), clue))
        answer_candidates = get_valid(state, answer_candidates)
        print("-" * 80)
        print()

if __name__ == "__main__":
    main()
