#include "model.hpp"
#include <string>
#include <iostream>

constexpr size_t WORD_LENGTH = 5u;
constexpr size_t ALPHABET_NUM = 26u;

using WordT = Word<WORD_LENGTH, ALPHABET_NUM>;

using std::string;

int main(int argc, char *argv[])
{
    using std::cin, std::cout, std::endl;

    if (argc <= 1)
    {
        cout << "Usage: " << argv[0] << " <word>.\n";
        return 1;
    }

    WordT answer(argv[1]);

    for (;;)
    {
        string guess;
        bool is_correct = true;
        cin >> guess;

        Clue clue(WordT(guess), answer);
        for (auto i = 0u; i < WORD_LENGTH; ++i)
        {
            if (clue[i] != 2)
            {
                is_correct = false;
                break;
            }
        }

        if (is_correct)
        {
            cout << "Bingo!\n";
            break;
        }
        for (auto i = 0u; i < WORD_LENGTH; ++i)
        {
            cout << clue[i];
        }
        cout << endl;
    }
    return 0;
}
