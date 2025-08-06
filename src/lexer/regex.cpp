// The regex class in the standard library is not sufficent for the purpose here. 
// I intend to use a standard regex differentiation method to simplify the lexing's memory usage.
// The general structure of the regex will be defined in a rather functional style
// Each regex will be defined by some construction upon one of nothing, some char, some other regex, 2 other regexes. 
// This will follow the standard regex patterns (as well as +). 
// (for now there will be no matching within a set of characters [0-9] will not be a valid regex for example)
// I also want to lazily evaluate these expressions. So take the instance of differentiating (r*) that will not be evaluated eagerly to 
// Concat(D(r)r*) untill some function is called on the regex that requires this.
// The instance where this will not be true is that the value stored in the regex will change at the point of required evaluation. 

#include <variant>
#include "../../include/aaronpiler/lexer/regex.h"

// typedef struct{}Epsilon;
// typedef struct{}EmptySet;
// typedef struct{char atom;}Atom;
// typedef struct{Regex* reg;}Star;
// typedef struct{Regex* left, *right;}Concat;
// typedef struct{Regex* left, *right;}Or;
// typedef struct{Regex* reg; char atom;}Diff;

// Implementation of static factory methods
Regex* Regex::makeEpsilon(){
    Regex* r = new Regex();
    r->val = Epsilon{};
    return r;
}

Regex* Regex::makeEmptySet(){
    Regex* r = new Regex();
    r->val = EmptySet{};
    return r;
}

Regex* Regex::makeAtom(char character){
    Regex* r = new Regex();
    r->val = Atom{character};
    return r;
}

Regex* Regex::makeStar(Regex* reg){
    Regex* r = new Regex();
    r->val = Star{reg};
    return r;
}

Regex* Regex::makeConcat(Regex* left, Regex* right){
    Regex* r = new Regex();
    r->val = Concat{left, right};
    return r;
}

Regex* Regex::makeOr(Regex* left, Regex* right){
    Regex* r = new Regex();
    r->val = Or{left, right};
    return r;
}

Regex* Regex::makeDiff(Regex* reg, char atom){
    Regex* r = new Regex();
    if (std::holds_alternative<Diff>(reg->val)) {
        Diff d = std::get<Diff>(reg->val);
        reg->derivative(d.atom);
    }
    r->val = Diff{reg, atom};
    return r;
}

// Implementation of instance methods
void Regex::derivative(char x) {
    if (std::holds_alternative<Epsilon>(val)) {
        val = EmptySet{};
        return;
    }
    if (std::holds_alternative<EmptySet>(val)) {
        val = EmptySet{};
        return;
    }
    if (std::holds_alternative<Atom>(val)) {
        Atom a = std::get<Atom>(val);
        if (a.atom == x) {
            val = Epsilon{};
        } else {
            val = EmptySet{};
        }
        return;
    }
    if (std::holds_alternative<Star>(val)) {
        Star s = std::get<Star>(val);
            Regex* c = s.reg->deepCopy();
            s.reg->derivative(x);
            val = Concat{s.reg, c};
        return;
    }
    if (std::holds_alternative<Concat>(val)) {
        Concat c = std::get<Concat>(val);
        c.left->derivative(x);
        if (c.left->nullable()) {
            // D(l·r) = D(l)·r + ε·D(r) if l is nullable
            Regex* r = c.right->deepCopy();
            c.right->derivative(x);
            Regex* n = makeConcat(c.left, r);
            val = Or{n, c.right};
            return;
        }
        // D(l·r) = D(l)·r if l is not nullable

        val = Concat{c.left, c.right};
        return;
    }
    if (std::holds_alternative<Or>(val)) {
        Or o = std::get<Or>(val);
        if (o.left && o.right) {
            o.left->derivative(x);
            o.right->derivative(x);
            val = Or{o.left, o.right};
        }
        return;
    }
    if (std::holds_alternative<Diff>(val)) {
        Diff d = std::get<Diff>(val);
        d.reg->derivative(d.atom);
        d.reg->derivative(x);
        val = d.reg->val;
    }
}

bool Regex::nullable() {
    if (std::holds_alternative<Epsilon>(val)) {
        return true;
    }
    if (std::holds_alternative<EmptySet>(val)) {
        return false;
    }
    if (std::holds_alternative<Atom>(val)) {
        return false;
    }
    if (std::holds_alternative<Star>(val)) {
        return true; // r* is always nullable
    }
    if (std::holds_alternative<Concat>(val)) {
        Concat c = std::get<Concat>(val);
        return c.left && c.right && c.left->nullable() && c.right->nullable();
    }
    if (std::holds_alternative<Or>(val)) {
        Or o = std::get<Or>(val);
        return o.left && o.right && (o.left->nullable() || o.right->nullable());
    }
    if (std::holds_alternative<Diff>(val)) {
        Diff d = std::get<Diff>(val);
        d.reg->derivative(d.atom);
        return d.reg->nullable();
    }
    return false;
}

Regex* Regex::deepCopy() const {
    Regex* copy = new Regex();
    
    if (std::holds_alternative<Epsilon>(val)) {
        copy->val = Epsilon{};
    } else if (std::holds_alternative<EmptySet>(val)) {
        copy->val = EmptySet{};
    } else if (std::holds_alternative<Atom>(val)) {
        Atom a = std::get<Atom>(val);
        copy->val = Atom{a.atom};
    } else if (std::holds_alternative<Star>(val)) {
        Star s = std::get<Star>(val);
        copy->val = Star{s.reg->deepCopy()};
    } else if (std::holds_alternative<Concat>(val)) {
        Concat c = std::get<Concat>(val);
        copy->val = Concat{c.left->deepCopy(), c.right->deepCopy()};
    } else if (std::holds_alternative<Or>(val)) {
        Or o = std::get<Or>(val);
        copy->val = Or{o.left->deepCopy(), o.right->deepCopy()};
    } else if (std::holds_alternative<Diff>(val)) {
        Diff d = std::get<Diff>(val);
        copy->val = Diff{d.reg->deepCopy(), d.atom};
    }
    
    return copy;
}

Regex:: ~Regex() {
    if (std::holds_alternative<Star>(val)) {
        Star s = std::get<Star>(val);
        delete s.reg;
    }
    if (std::holds_alternative<Concat>(val)) {
        Concat c = std::get<Concat>(val);
        delete c.left;
        delete c.right;
    }
    if (std::holds_alternative<Or>(val)) {
        Or o = std::get<Or>(val);
        delete o.left;
        delete o.right;
    }
    if (std::holds_alternative<Diff>(val)) {
        Diff d = std::get<Diff>(val);
        delete d.reg;
    }
    // delete this;
    return;
}