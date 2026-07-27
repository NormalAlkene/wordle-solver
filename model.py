#!/usr/bin/env python3

import numpy as np
from typing import Any
from numpy.typing import NDArray
from numba import njit

DEFAULT_ALPHABET_NUM = 26
DEFAULT_WORD_LENGTH = 5

CLUE_GRAY = 0
CLUE_YELLOW = 1
CLUE_GREEN = 2

class Word:
    word: NDArray[np.uint32]
    bincount: NDArray[np.uint32]

    def __init__(self,
                 word: NDArray[np.uint32] = np.zeros(DEFAULT_WORD_LENGTH, dtype=np.uint32),
                 alphabet_num: int = DEFAULT_ALPHABET_NUM):
        self.word = word
        self.bincount = np.bincount(word, minlength=alphabet_num)

    def __len__(self) -> int:
        return len(self.word)

    def alphabet_num(self) -> int:
        return len(self.bincount)

    def __getitem__(self, i: Any) -> np.uint32:
        return self.word[i]

    def __eq__(self, other: Word) -> bool:
        return np.all(self.word == other.word)

    def __hash__(self) -> int:
        return np.dot(self.word, self.alphabet_num() ** np.arange(len(self.word))).item()

    @classmethod
    def from_eng_str(cls, word: str, /) -> Word:
        if not word.isalpha():
            raise ValueError(f"The word \"{word}\" is not a valid English word")
        return cls(word=np.frombuffer(word.encode(), dtype=np.uint8) - ord("a"), alphabet_num=DEFAULT_ALPHABET_NUM)

    def __repr__(self) -> str:
        return "".join([ chr(ch + ord("a")) for ch in self.word ])

class Clue:
    word: Word
    clue: NDArray[np.uint32]

    def __init__(self, word: Word, clue: str | NDArray[np.uint32] | list[int]):
        self.word = word
        if isinstance(clue, NDArray):
            self.clue = clue
        elif isinstance(clue, str):
            if len(clue.strip()) != len(word)):
                raise ValueError(f"The length of clue string \"{clue}\" must be the same as of word")
            np.array([int(it) for it in list(clue)])

    def __len__(self) -> int:
        return len(self.word)

    def alphabet_num(self) -> int:
        return len(self.word.bincount)

    def __getitem__(self, i: Any) -> np.uint32:
        return self.clue[i]

    def __eq__(self, other: Clue) -> bool:
        return np.all(self.word == other.word) and np.all(self.clue == other.clue)

    def __hash__(self) -> int:
        return np.dot(
            np.multiply(self.word.word, self.clue + 1),
            (self.alphabet_num() * 3) ** np.arange(len(self.word))
        ).item()

    @classmethod
    #@njit
    def from_words(cls, guess: Word, answer: Word):
        ret = cls(guess, np.zeros_like(guess.word))

        # Green
        matching = (guess.word == answer.word)
        ret.clue[matching] = CLUE_GREEN
        bincount = answer.bincount.copy()
        for i in range(len(answer)):
            if ret.clue[i] == CLUE_GRAY and bincount[ret.word[i]] > 0:
                ret.clue[i] = CLUE_YELLOW
                bincount[ret.word[i]] -= 1

        return ret

class State:
    _possible: NDArray[np.bool_]
    _min_counts: NDArray[np.uint32]
    _max_counts: NDArray[np.uint32]

    def __init__(
        self,
        *,
        word_length: int = DEFAULT_WORD_LENGTH,
        alphabet_num: int = DEFAULT_ALPHABET_NUM
    ):
        self._possible = np.ones([word_length, alphabet_num], np.bool_)
        self._min_counts = np.zeros([alphabet_num], dtype=np.uint32)
        self._max_counts = np.full([alphabet_num], word_length, dtype=np.uint32)

    def word_length(self) -> int:
        return self._possible.shape[0]

    def alphabet_num(self) -> int:
        return self._possible.shape[1]

    #@njit
    def check(self, target: Word) -> bool:
        if len(target.word) != self.word_length():
            raise ValueError("The length of the word must be the same as the state's")

        length = self.word_length()

        # 1. Check possiblity
        if not self._possible[np.arange(len(target)), target.word].all():
            return False

        # 2. Check count
        sums = np.bincount(target.word, minlength=self.alphabet_num())
        if np.any(sums < self._min_counts) or np.any(sums > self._max_counts):
            return False

        return True

    def __and__(self, other: State) -> State:
        ret = State(word_length=self.word_length(), alphabet_num=self.alphabet_num())
        ret._possible = self._possible & other._possible
        ret._min_counts = np.maximum(self._min_counts, other._min_counts)
        ret._max_counts = np.minimum(self._max_counts, other._max_counts)
        #np.clip(ret._max_counts, None, self._possible.shape[0] - np.sum(ret._min_counts), out=ret._max_counts)
        return ret

    @classmethod
    #@njit
    def from_clue(cls, clue: Clue) -> "State":
        length = len(clue.word)
        alphabet_num = clue.word.alphabet_num()
        ret = cls(word_length=length, alphabet_num=alphabet_num)

        for i in range(length):
            if clue[i] == CLUE_GREEN:
                ret._possible[i] = np.False_
                ret._possible[i, clue.word[i]] = np.True_
                ret._min_counts[clue.word[i]] += 1

        for i in range(length):
            if clue[i] == CLUE_YELLOW:
                ret._possible[i, clue.word[i]] = np.False_
                ret._min_counts[clue.word[i]] += 1
            elif clue[i] ==  CLUE_GRAY:
                ret._possible[i:, clue.word[i]] = np.False_
                ret._max_counts[clue.word[i]] = ret._min_counts[clue.word[i]]

        #np.clip(ret._max_counts, 0, length - np.sum(ret._min_counts), out=ret._max_counts)

        return ret

