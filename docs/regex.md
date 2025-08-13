# Regex Class Documentation

## Overview

The `Regex` class implements a functional-style regular expression system designed specifically for lexing purposes. It uses **regex differentiation** to simplify memory usage during lexical analysis by avoiding eager evaluation of complex expressions.

## Design Philosophy

### Functional Style
The regex implementation follows a functional programming approach where each regex is constructed from primitive operations:
- **Epsilon** (ε): Matches empty string
- **EmptySet** (∅): Matches nothing
- **Atom** (a): Matches a single character
- **Star** (r*): Zero or more repetitions
- **Concat** (r₁·r₂): Concatenation of two regexes
- **Or** (r₁|r₂): Alternation of two regexes
- **Diff** (r\a): Difference (r but not matching 'a')

### Lazy Evaluation
The system implements lazy evaluation to avoid unnecessary computation:
- Derivatives are not computed until required
- Complex expressions like `D(r*)` are not eagerly expanded to `Concat(D(r), r*)`
- Values are only evaluated when a function requires the result

## Class Structure

### Header File: `include/aaronpiler/lexer/regex.h`

```cpp
class Regex {
public:
    using RegexVariant = std::variant<Epsilon, EmptySet, Atom, Star, Concat, Or, Diff>;
    
    // Constructors and destructor
    Regex() = default;
    ~Regex();
    
    // Static factory methods
    static Regex* makeEpsilon();
    static Regex* makeEmptySet();
    static Regex* makeAtom(char x);
    static Regex* makeStar(Regex* r);
    static Regex* makeConcat(Regex* l, Regex* r);
    static Regex* makeOr(Regex* l, Regex* r);
    static Regex* makeDiff(Regex* r, char x);
    
    // Instance methods
    void derivative(char x);
    bool nullable();
    Regex* deepCopy() const;

private:
    RegexVariant val;
};
```

### Variant Types

Each regex type is represented as a struct within the `std::variant`:

```cpp
typedef struct{}Epsilon;                    // ε
typedef struct{}EmptySet;                   // ∅
typedef struct{char atom;}Atom;             // a
typedef struct{Regex* reg;}Star;            // r*
typedef struct{Regex* left, *right;}Concat; // r₁·r₂
typedef struct{Regex* left, *right;}Or;     // r₁|r₂
typedef struct{Regex* reg; char atom;}Diff; // r\a
```

## Factory Methods

### Creating Primitive Regexes

```cpp
// Create epsilon regex (matches empty string)
Regex* epsilon = Regex::makeEpsilon();

// Create empty set regex (matches nothing)
Regex* empty = Regex::makeEmptySet();

// Create atom regex (matches single character)
Regex* char_a = Regex::makeAtom('a');
```

### Creating Compound Regexes

```cpp
// Create star regex (zero or more repetitions)
Regex* star = Regex::makeStar(some_regex);

// Create concatenation (r₁ followed by r₂)
Regex* concat = Regex::makeConcat(regex1, regex2);

// Create alternation (r₁ or r₂)
Regex* alternation = Regex::makeOr(regex1, regex2);

// Create difference (r but not matching 'a')
Regex* diff = Regex::makeDiff(some_regex, 'a');
```

## Core Methods

### `derivative(char x)`
Computes the derivative of the regex with respect to character `x`. This is the core operation that enables regex matching through differentiation.

**Mathematical Definition:**
- D(ε) = ∅
- D(∅) = ∅  
- D(a) = ε if a = x, else ∅
- D(r*) = D(r)·r*
- D(r₁·r₂) = D(r₁)·r₂ + ε·D(r₂) if r₁ is nullable
- D(r₁|r₂) = D(r₁)|D(r₂)
- D(r\a) = D(r)\a

### `nullable()`
Returns `true` if the regex can match the empty string, `false` otherwise.

**Mathematical Definition:**
- nullable(ε) = true
- nullable(∅) = false
- nullable(a) = false
- nullable(r*) = true
- nullable(r₁·r₂) = nullable(r₁) ∧ nullable(r₂)
- nullable(r₁|r₂) = nullable(r₁) ∨ nullable(r₂)
- nullable(r\a) = nullable(r)

### `deepCopy()`
Creates a deep copy of the regex, including all nested regexes. Essential for avoiding shared state during derivative computation.

## Usage Examples

### Basic Regex Construction

```cpp
// Create regex for "ab*"
Regex* a = Regex::makeAtom('a');
Regex* b = Regex::makeAtom('b');
Regex* b_star = Regex::makeStar(b);
Regex* ab_star = Regex::makeConcat(a, b_star);

// Create regex for "(a|b)*"
Regex* a_or_b = Regex::makeOr(a, b);
Regex* a_or_b_star = Regex::makeStar(a_or_b);
```

### Matching Process

```cpp
// Example: Match "ab" against regex "ab*"
Regex* regex = /* create ab* regex */;
std::string input = "ab";

// Process each character
for (char c : input) {
    regex->derivative(c);
}

// Check if we can accept
if (regex->nullable()) {
    std::cout << "Match!" << std::endl;
} else {
    std::cout << "No match." << std::endl;
}
```

## Memory Management

### Ownership Model
- Factory methods return `Regex*` (raw pointers)
- Caller is responsible for memory management
- `deepCopy()` creates new instances that must be deleted
- Destructor handles cleanup of nested regexes

### Best Practices
```cpp
// Always clean up regexes
Regex* r1 = Regex::makeAtom('a');
Regex* r2 = Regex::makeAtom('b');
Regex* combined = Regex::makeConcat(r1, r2);

// Use the combined regex...

// Clean up (in reverse order of creation)
delete combined;  // This should handle r1 and r2 if they're not shared
```

## Limitations

### Current Restrictions
1. **No character classes**: `[0-9]`, `[a-z]` are not supported
2. **No quantifiers**: `+`, `?`, `{n,m}` are not implemented
3. **No anchors**: `^`, `$` are not supported
4. **No escaping**: Special characters are not escaped

### Future Extensions
- Character class support
- Additional quantifiers
- Anchoring support
- Escaping mechanism
- Unicode support

## Performance Characteristics

### Time Complexity
- **Derivative computation**: O(1) for primitive operations
- **Nullable check**: O(1) for primitive operations
- **Deep copy**: O(n) where n is the size of the regex tree

### Space Complexity
- **Memory usage**: Proportional to regex complexity
- **Lazy evaluation**: Reduces memory for complex expressions
- **Shared subexpressions**: Not currently optimized

## Integration with Lexer

The regex class is designed to work with a lexer that:
1. Creates regexes for each token pattern
2. Computes derivatives for each input character
3. Tracks which patterns are still viable
4. Accepts the longest matching pattern

This approach provides efficient lexical analysis with minimal memory overhead compared to traditional regex engines. 