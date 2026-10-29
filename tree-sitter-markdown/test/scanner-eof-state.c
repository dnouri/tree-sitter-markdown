#include <stdio.h>
#include "../src/scanner.c"

static bool at_eof(const TSLexer *lexer) {
    (void)lexer;
    return true;
}

int main(void) {
    size_t cases = 0;
    const uint8_t matches[] = {0, 1, 255};
    for (unsigned state = 0; state <= UINT8_MAX; state++) {
        for (unsigned m = 0; m < sizeof(matches) / sizeof(matches[0]); m++) {
            // Empty stack, final pop, nonfinal pop, simulation, explicit EOF.
            for (unsigned variant = 0; variant < 5; variant++) {
                Block blocks[2] = {INDENTED_CODE_BLOCK, INDENTED_CODE_BLOCK};
                Scanner s = {0};
                s.open_blocks.items = blocks;
                s.open_blocks.capacity = 2;
                s.open_blocks.size = variant == 0 ? 0 : variant == 2 ? 2 : 1;
                s.state = state;
                s.matched = matches[m];
                s.indentation = 3;
                s.column = 2;
                s.fenced_code_block_delimiter_length = 4;
                s.simulate = variant == 3;
                TSLexer lexer = {.eof = at_eof};
                bool valid_symbols[NUM_TOKEN_TYPES] = {0};
                valid_symbols[TOKEN_EOF] = variant == 4;
                size_t old_size = s.open_blocks.size;
                bool actual_pop = old_size > 0 && !s.simulate && !valid_symbols[TOKEN_EOF];
                bool reset = actual_pop && old_size == 1;
                bool result = scan(&s, &lexer, valid_symbols);
                assert(result == (variant != 0));
                assert(s.open_blocks.size == old_size - actual_pop);
                assert(s.state == (reset ? (state & ~(STATE_MATCHING | STATE_CLOSE_BLOCK)) : state));
                assert(s.matched == (reset ? 0 : matches[m]));
                assert(s.indentation == 3 && s.column == 2);
                assert(s.fenced_code_block_delimiter_length == 4);
                assert(s.simulate == (variant == 3));
                if (result)
                    assert(lexer.result_symbol == (variant == 4 ? TOKEN_EOF : BLOCK_CLOSE));
                if (reset) {
                    char buffer[TREE_SITTER_SERIALIZATION_BUFFER_SIZE];
                    unsigned length = serialize(&s, buffer);
                    Scanner restored = {0};
                    deserialize(&restored, buffer, length);
                    assert(restored.open_blocks.size == 0);
                    assert(!(restored.state & (STATE_MATCHING | STATE_CLOSE_BLOCK)));
                    assert(restored.matched == 0);
                    assert(restored.state == s.state);
                }
                cases++;
            }
        }
    }
    printf("Scanner EOF state: %zu cases passed\n", cases);
    return 0;
}
