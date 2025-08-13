#include <iostream>
#include <cassert>
#include "aaronpiler/lexer/regex.h"

// --- Basic Tests ---

void test_epsilon() {
    Regex* r = Regex::makeEpsilon();
    assert(r->nullable() == true);
    r->derivative('a');
    assert(r->nullable() == false); // D(epsilon) = ∅
    delete r;
}

void test_atom_match() {
    Regex* r = Regex::makeAtom('a');
    assert(r->nullable() == false);
    r->derivative('a');
    assert(r->nullable() == true); // D(a) = ε
    delete r;
}

void test_atom_no_match() {
    Regex* r = Regex::makeAtom('b');
    r->derivative('a');
    assert(r->nullable() == false); // D(b, a) = ∅
    delete r;
}

void test_star_nullable() {
    Regex* r = Regex::makeStar(Regex::makeAtom('a'));
    assert(r->nullable() == true); // a* is always nullable
    delete r;
}

void test_star_derivative() {
    Regex* r = Regex::makeStar(Regex::makeAtom('a'));
    r->derivative('a');
    assert(r->nullable() == true); 
    delete r;
}
void test_star_derivative_other() {
    Regex* r = Regex::makeStar(Regex::makeAtom('a'));
    r->derivative('b');
    assert(r->nullable() == false); 
    delete r;
}

void test_concat_nullable() {
    Regex* r = Regex::makeConcat(
        Regex::makeEpsilon(),
        Regex::makeEpsilon()
    );
    assert(r->nullable() == true);
    delete r;
}

void test_concat_not_nullable() {
    Regex* r = Regex::makeConcat(
        Regex::makeEpsilon(),
        Regex::makeAtom('b')
    );
    assert(r->nullable() == false);
    delete r;
}

void test_or_nullable() {
    Regex* r = Regex::makeOr(
        Regex::makeEpsilon(),
        Regex::makeAtom('x')
    );
    assert(r->nullable() == true);
    delete r;
}

void test_diff_lazy_evaluation() {
    Regex* r = Regex::makeDiff(
        Regex::makeAtom('a'),
        'a'
    );
    assert(r->nullable() == true); // D(a, a) = ε
    delete r;
}
// medium difficulty
void test_diff_concat_derivative() {
    Regex* r = Regex::makeConcat(
        Regex::makeAtom('a'),
        Regex::makeAtom('b')
    );

    // D(a·b, 'a') = ε·b = b
    r->derivative('a');
    assert(r->nullable() == false);

    // D(b, 'b') = ε
    r->derivative('b');
    assert(r->nullable() == true);

    delete r;
}



// --- Non-Trivial / Compositional Tests ---

void test_star_concat_derivative() {
    Regex* r =Regex::makeDiff( Regex::makeConcat(
        Regex::makeStar(Regex::makeAtom('a')),
        Regex::makeAtom('b')), 'a'
    );

    assert(r->nullable() == false);
    std::cout<< "it passed correctly just segfaulted after";
    delete r;
}

void test_or_concat_derivative() {
    Regex* r = Regex::makeOr(
        Regex::makeAtom('a'),
        Regex::makeConcat(
            Regex::makeAtom('b'),
            Regex::makeAtom('c')
        )
    );

    r->derivative('b'); // becomes ε·c = c
    assert(r->nullable() == false);

    r->derivative('c'); // should reduce to ε
    assert(r->nullable() == true);

    delete r;
}

void test_star_or_concat_derivative() {
    // (a|b)* c
    Regex* r = Regex::makeConcat(
        Regex::makeStar(
            Regex::makeOr(
                Regex::makeAtom('a'),
                Regex::makeAtom('b')
            )
        ),
        Regex::makeAtom('c')
    );

    r->derivative('a');
    assert(r->nullable() == false);

    r->derivative('b');
    assert(r->nullable() == false);

    r->derivative('c');
    assert(r->nullable() == true);

    delete r;
}

void test_nested_or_star() {
    // (ab|c)*. 

    Regex* r = Regex::makeStar(
        Regex::makeOr(
            Regex::makeConcat(
                Regex::makeAtom('a'), 
                Regex::makeAtom('b')), 
            Regex::makeAtom('c')));
    r->derivative('a'); // should become b·r
    assert(r->nullable() == false);
    r->derivative('b'); // now should be r again
    assert(r->nullable() == true); // r* is always nullable


    delete r;
}

void test_alternating_letters_star() {
    // Regex: (ab|c)*d
    Regex* r = Regex::makeConcat(
        Regex::makeStar(
            Regex::makeOr(
                Regex::makeConcat(
                    Regex::makeAtom('a'),
                    Regex::makeAtom('b')
                ),
                Regex::makeAtom('c')
            )
        ),
        Regex::makeAtom('d')
    );

    // Match: "abcd"
    r->derivative('a'); // inside ab
    assert(r->nullable() == false);

    r->derivative('b'); // completes "ab", loops back to star
    assert(r->nullable() == false);

    r->derivative('c'); // another star iteration
    assert(r->nullable() == false);

    r->derivative('d'); // final 'd'
    assert(r->nullable() == true);

    delete r;
}


void test_optional_then_required() {
    // Regex: (a|ε)b
    Regex* r = Regex::makeConcat(
        Regex::makeOr(
            Regex::makeAtom('a'),
            Regex::makeEpsilon()
        ),
        Regex::makeAtom('b')
    );

    // Match with no 'a'
    r->derivative('b');
    assert(r->nullable() == true);

    // Reset and match with 'a'
    delete r;
    r = Regex::makeConcat(
        Regex::makeOr(
            Regex::makeAtom('a'),
            Regex::makeEpsilon()
        ),
        Regex::makeAtom('b')
    );

    r->derivative('a');
    assert(r->nullable() == false);

    r->derivative('b');
    assert(r->nullable() == true);

    delete r;
}
// --- Advanced / Complex Simulation Tests ---

void test_email_regex_simulation() {
    // Simulated regex: [a-z]+@[a-z]+.com
    Regex* r = Regex::makeConcat(
        Regex::makeStar( // [a-z]+
            Regex::makeOr(
                Regex::makeAtom('a'),
                Regex::makeOr(
                    Regex::makeAtom('b'),
                    Regex::makeAtom('c')
                )
            )
        ),
        Regex::makeConcat(
            Regex::makeAtom('@'),
            Regex::makeConcat(
                Regex::makeStar(
                    Regex::makeOr(
                        Regex::makeAtom('a'),
                        Regex::makeAtom('b')
                    )
                ),
                Regex::makeConcat(
                    Regex::makeAtom('.'),
                    Regex::makeConcat(
                        Regex::makeAtom('c'),
                        Regex::makeConcat(
                            Regex::makeAtom('o'),
                            Regex::makeAtom('m')
                        )
                    )
                )
            )
        )
    );

    const char* input = "abc@ab.com";
    for (const char* p = input; *p; ++p) {
        r->derivative(*p);
    }
    assert(r->nullable() == true);

    delete r;
}

void test_json_string_regex_simulation() {
    // Simulated regex: " ([a-z]|[0-9])* "
    Regex* r = Regex::makeConcat(
        Regex::makeAtom('"'),
        Regex::makeConcat(
            Regex::makeStar(
                Regex::makeOr(
                    Regex::makeAtom('a'),
                    Regex::makeOr(
                        Regex::makeAtom('b'),
                        Regex::makeAtom('1')
                    )
                )
            ),
            Regex::makeAtom('"')
        )
    );

    const char* input = "\"ab1ba\"";
    for (const char* p = input; *p; ++p) {
        r->derivative(*p);
    }
    assert(r->nullable() == true);

    delete r;
}

void test_nested_parentheses_regex_simulation() {
    // Simulated regex: '(' ('a'| '(' 'a' ')') ')'
    Regex* r = Regex::makeConcat(
        Regex::makeAtom('('),
        Regex::makeConcat(
            Regex::makeOr(
                Regex::makeAtom('a'),
                Regex::makeConcat(
                    Regex::makeAtom('('),
                    Regex::makeConcat(
                        Regex::makeAtom('a'),
                        Regex::makeAtom(')')
                    )
                )
            ),
            Regex::makeAtom(')')
        )
    );

    const char* input = "(a)";
    for (const char* p = input; *p; ++p) {
        r->derivative(*p);
    }
    assert(r->nullable() == true);

    delete r;

    // Nested case: ((a))
    r = Regex::makeConcat(
        Regex::makeAtom('('),
        Regex::makeConcat(
            Regex::makeConcat(
                Regex::makeAtom('('),
                Regex::makeConcat(
                    Regex::makeAtom('a'),
                    Regex::makeAtom(')')
                )
            ),
            Regex::makeAtom(')')
        )
    );
    const char* input2 = "((a))";
    for (const char* p = input2; *p; ++p) {
        r->derivative(*p);
    }
    assert(r->nullable() == true);

    delete r;
}

void test_complex_number_regex_simulation() {

    Regex* r = Regex::makeConcat(
        Regex::makeOr(
            Regex::makeEpsilon(),
            Regex::makeOr(
                Regex::makeAtom('+'),
                Regex::makeAtom('-')
            )
        ),
        Regex::makeConcat(
            Regex::makeStar(Regex::makeAtom('1')), // pretend only '1' is a digit for simplicity
            Regex::makeOr(
                Regex::makeEpsilon(),
                Regex::makeConcat(
                    Regex::makeAtom('.'),
                    Regex::makeStar(Regex::makeAtom('1'))
                )
            )
        )
    );

    const char* input = "-111.11";
    for (const char* p = input; *p; ++p) {
        r->derivative(*p);
    }
    assert(r->nullable() == true);

    delete r;
}



// --- Main Test Driver ---

int main() {
    test_epsilon();
    std::cout << "test_epsilon passed!" << std::endl;
    
    test_atom_match();
    std::cout << "test_atom_match passed!" << std::endl;
    
    test_atom_no_match();
    std::cout << "test_atom_no_match passed!" << std::endl;
    
    test_star_nullable();
    std::cout << "test_star_nullable passed!" << std::endl;
    
    test_star_derivative();
    std::cout << "test_star_derivative passed!" << std::endl;
    
    test_star_derivative_other();
    std::cout << "test_star_derivative_other passed" <<std::endl;

    test_concat_nullable();
    std::cout << "test_concat_nullable passed!" << std::endl;
    
    test_concat_not_nullable();
    std::cout << "test_concat_not_nullable passed!" << std::endl;
    
    test_or_nullable();
    std::cout << "test_or_nullable passed!" << std::endl;
    
    test_diff_lazy_evaluation();
    std::cout << "test_diff_lazy_evaluation passed!" << std::endl;

    test_diff_concat_derivative();
    std::cout << "test_diff_concat_derivative passed!" << std::endl;


    test_star_concat_derivative();
    std::cout << "test_star_concat_derivative passed!" << std::endl;
    
    test_or_concat_derivative();
    std::cout << "test_or_concat_derivative passed!" << std::endl;
    
    test_star_or_concat_derivative();
    std::cout << "test_star_or_concat_derivative passed!" << std::endl;
    
    test_nested_or_star();
    std::cout << "test_nested_or_star passed!" << std::endl;

    test_alternating_letters_star();
    std::cout << "test_alternating_letters_star passed!" << std::endl;

    test_optional_then_required();
    std::cout << "test_optional_then_required passed!" << std::endl;
    // Complex real-world regex tests
    
    test_email_regex_simulation();
    std::cout << "test_email_regex_simulation passed!" << std::endl;
    
    test_json_string_regex_simulation();
    std::cout << "test_json_string_regex_simulation passed!" << std::endl;
    
    test_nested_parentheses_regex_simulation();
    std::cout << "test_nested_parentheses_regex_simulation passed!" << std::endl;
    
    test_complex_number_regex_simulation();
    std::cout << "test_complex_number_regex_simulation passed!" << std::endl;

    std::cout << "✅ All tests passed!" << std::endl;
    return 0;
}
