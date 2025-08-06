#ifndef REGEX_H
#define REGEX_H

#include <variant>

// Forward declaration
class Regex;

// Forward declarations for the variant types
typedef struct{}Epsilon;
typedef struct{}EmptySet;
typedef struct{char atom;}Atom;
typedef struct{Regex* reg;}Star;
typedef struct{Regex* left, *right;}Concat;
typedef struct{Regex* left, *right;}Or;
typedef struct{Regex* reg; char atom;}Diff;

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
    static Regex* makeOR(Regex* l, Regex* r);
    static Regex* makeDiff(Regex* r, char x);
    
    // Instance methods
    void derivative(char x);
    bool nullable();
    Regex* deepCopy() const;

private:
    RegexVariant val;
};

#endif 
