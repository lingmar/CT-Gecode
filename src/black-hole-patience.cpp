/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Mikael Lagerkvist <lagerkvist@gecode.org>
 *
 *  Copyright:
 *     Mikael Lagerkvist, 2006
 *
 *  Last modified:
 *     $Date: 2016-04-19 17:19:45 +0200 (Tue, 19 Apr 2016) $ by $Author: schulte $
 *     $Revision: 14967 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
 
#include <gecode/driver.hh>
#include <gecode/int.hh>
 
#include <vector>
#include <algorithm>
#include <sstream>

#include "compact-table.hpp"
 
using namespace Gecode;
 
namespace {
  using std::vector;
 
  vector<vector<int> > layout;
  vector<int> layer, pile;
 
  void generate(int seed) {
    // The layout consists of 17 piles of 3 cards each
    layout = vector<vector<int> >(17, vector<int>(3));
    // Deck without the Ace of Spades
    vector<int> deck(51);
    for (int i = 51; i--; ) deck[i] = i+1;
    Support::RandomGenerator rnd(seed+1);
    std::random_shuffle(deck.begin(), deck.end(), rnd);
 
    // Place cards from deck
    int pos = 0;
    for (int i = 17; i--; )
      for (int j = 3; j--; )
        layout[i][j] = deck[pos++];
 
    // Location-information for each card
    layer = vector<int>(52);
    pile  = vector<int>(52);
    for (int i = 17; i--; ) {
      for (int j = 3; j--; ) {
        layer[layout[i][j]] = j;
        pile[ layout[i][j]] = i;
      }
    }
  }
}
 
class BlackHole : public Script {
protected:
  IntVarArray x, 
    y; 
 
  std::string
  card(int val) const {
    const char* suit = "SCHD";
    std::ostringstream o;
    o << std::setw(2) << (1 + (val%13)) << suit[val/13];
    return o.str();
  }
 
public:
  enum {
    SYMMETRY_NONE,       
    SYMMETRY_CONDITIONAL 
  };
  enum {
    PROPAGATION_REIFIED,  
    PROPAGATION_DFA,      
    PROPAGATION_TUPLE_SET 
  };
  BlackHole(const SizeOptions& opt)
    : Script(opt), x(*this, 52, 0,51), y(*this, 52, 0,51) {
    // Black ace at bottom
    rel(*this, x[0], IRT_EQ, 0);
 
    // x is order and y is placement
    channel(*this, x, y, opt.ipl());
 
    // The placement rules: the absolute value of the difference
    // between two consecutive cards is 1 or 12.
    if (opt.propagation() == PROPAGATION_REIFIED) {
      // Build table for accessing the rank of a card
      IntArgs modtable(52);
      for (int i = 0; i < 52; ++i) {
        modtable[i] = i%13;
      }
      for (int i = 0; i < 51; ++i) {
        IntVar x1(*this, 0, 12), x2(*this, 0, 12);
        element(*this, modtable, x[i], x1);
        element(*this, modtable, x[i+1], x2);
        const int dr[2] = {1, 12};
        IntVar diff(*this, IntSet(dr, 2));
        rel(*this, abs(x1-x2) == diff, IPL_DOM);
      }
    } else if (opt.propagation() == PROPAGATION_DFA) {
      // Build table for allowed tuples
      REG expression;
      for (int r = 13; r--; ) {
        for (int s1 = 4; s1--; ) {
          for (int s2 = 4; s2--; ) {
            for (int i = -1; i <= 1; i+=2) {
              REG r1 = REG(r+13*s1);
              REG r2 = REG((r+i+52+13*s2)%52);
              REG r = r1 + r2;
              expression |= r;
            }
          }
        }
      }
      DFA table(expression);
 
      for (int i = 51; i--; )
        extensional(*this, IntVarArgs() << x[i] << x[i+1], table);
 
    } else { // opt.propagation() == PROPAGATION_TUPLE_SET)
      // Build table for allowed tuples
      TupleSet tupleSet;
      for (int r = 13; r--; )
        for (int s1 = 4; s1--; )
          for (int s2 = 4; s2--; )
            for (int i = -1; i <= 1; i+=2) {
              tupleSet.add(IntArgs(2, r+13*s1, (r+i+52+13*s2)%52));
            }
      tupleSet.finalize();
 
      for (int i = 51; i--; ) {
        switch (opt.model()) {
        case 0: {
          extensional(*this, IntVarArgs() << x[i] << x[i+1], tupleSet);
          break;
        }
        case 1: {
          extensional2(*this, IntVarArgs() << x[i] << x[i+1], tupleSet);
          break;
        }
        default:
          break;
        }

      }
    }
 
    // A card must be played before the one under it.
    for (int i = 17; i--; )
      for (int j = 2; j--; )
        rel(*this, y[layout[i][j]] < y[layout[i][j+1]]);
 
    // Compute and break the conditional symmetries that are dependent
    // on the current layout.
    // Two cards with the same rank but different suits are symmetric
    // with respect to their placement in the black hole if changing
    // their order does not affect any other card.
    if (opt.symmetry() == SYMMETRY_CONDITIONAL) {
      // For all ranks
      for (int r = 13; r--; ) {
        // For all pairs of suits
        for (int s1 = 4; s1--; ) {
          for (int s2 = s1; s2--; ) {
            int c1 = 13*s1 + r,
              c2 = 13*s2 + r;
            // The ace of spades is already placed
            if (c1 == 0 || c2 == 0) continue;
            // Piles are handled by the rules of the game
            if (pile[c1] == pile[c2]) continue;
            // Fix the right order of the cards
            int o1 = c1, o2 = c2;
            if (pile[c1] > pile[c2] && layer[c2] >= layer[c1])
              std::swap(o1, o2);
            // cond is the condition for the symmetry
            BoolVarArgs ba;
            // Both cards played after the ones on top of them
            for (int i = 0; i < layer[o1]; ++i)
              ba << expr(*this, (y[layout[pile[o1]][i]] < y[o2]));
            for (int i = 0; i < layer[o2]; ++i)
              ba << expr(*this, (y[layout[pile[o2]][i]] < y[o1]));
            // Both cards played before the ones under them
            for (int i = layer[o1]+1; i < 3; ++i)
              ba << expr(*this, (y[o2] < y[layout[pile[o1]][i]]));
            for (int i = layer[o2]+1; i < 3; ++i)
              ba << expr(*this, (y[o1] < y[layout[pile[o2]][i]]));
            // Cond holds when all the above holds
            BoolVar cond(*this, 0, 1);
            rel(*this, BOT_AND, ba, cond);
 
            // If cond is fulfilled, then we can order the cards
            // cond -> (y[o1] < y[o2])
            rel(*this, !cond || (y[o1] < y[o2]));
          }
        }
      }
    }
 
    // Install custom brancher
    branch(*this, x, INT_VAR_NONE(), INT_VAL(&val));
  }
  static int val(const Space&, IntVar x, int) {
    int v = -1;
    int w = 4;
    for (IntVarValues vals(x); vals(); ++vals)
      if (layer[vals.val()] < w) {
        v = vals.val();
        if ((w = layer[vals.val()]) == 0)
          break;
      }
    assert(v >= 1 && v < 52);
    return v;
  }
  virtual void
  print(std::ostream& os) const {
    os << "Layout:" << std::endl;
    for (int i = 0; i < 17; i++) {
      for (int j = 0; j < 3; j++)
        os << card(layout[i][j]) << " ";
      if ((i+1) % 3 == 0)
        os << std::endl;
      else
        os << "  \t";
    }
    os << std::endl << std::endl;
 
    os << "Solution:" << std::endl;
    for (int i = 0; i < 52; ++i) {
      if (x[i].assigned())
        os << card(x[i].val()) << " ";
      else
        os << "   ";
      if ((i + 1) % 13 == 0)
        os << std::endl;
    }
    os << std::endl;
    os << std::endl;
  }
 
  BlackHole(bool share, BlackHole& s) : Script(share,s) {
    x.update(*this, share, s.x);
    y.update(*this, share, s.y);
  }
  virtual Space*
  copy(bool share) {
    return new BlackHole(share,*this);
  }
};
 
int
main(int argc, char* argv[]) {
  SizeOptions opt("Black Hole patience");
  opt.symmetry(BlackHole::SYMMETRY_CONDITIONAL);
  opt.symmetry(BlackHole::SYMMETRY_NONE,"none",
               "no symmetry breaking");
  opt.symmetry(BlackHole::SYMMETRY_CONDITIONAL,"conditional",
               "break conditional symmetries");
  opt.propagation(BlackHole::PROPAGATION_TUPLE_SET);
  opt.propagation(BlackHole::PROPAGATION_REIFIED,
                  "reified", "use reified propagation");
  opt.propagation(BlackHole::PROPAGATION_DFA,
                  "dfa", "use DFA-based extensional propagation");
  opt.propagation(BlackHole::PROPAGATION_TUPLE_SET,
                  "tuple-set", "use TupleSet-based extensional propagation");
  opt.ipl(IPL_DOM);
  opt.model(0, "g", "extensional");
  opt.model(1, "c", "compact table");
  opt.parse(argc,argv);
  // Generates the new board
  generate(opt.size());
  Script::run<BlackHole,DFS,SizeOptions>(opt);
  return 0;
}
 
//  // STATISTICS: example-any
// /* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
// /*
//   *  Main authors:
//   *     Mikael Lagerkvist <lagerkvist@gecode.org>
//   *
//   *  Copyright:
//   *     Mikael Lagerkvist, 2006
//   *
//   *  Last modified:
//   *     $Date: 2015-03-17 16:09:39 +0100 (Tue, 17 Mar 2015) $ by $Author: schulte $
//   *     $Revision: 14447 $
//   *
//   *  This file is part of Gecode, the generic constraint
//   *  development environment:
//   *     http://www.gecode.org
//   *
//   *  Permission is hereby granted, free of charge, to any person obtaining
//   *  a copy of this software and associated documentation files (the
//   *  "Software"), to deal in the Software without restriction, including
//   *  without limitation the rights to use, copy, modify, merge, publish,
//   *  distribute, sublicense, and/or sell copies of the Software, and to
//   *  permit persons to whom the Software is furnished to do so, subject to
//   *  the following conditions:
//   *
//   *  The above copyright notice and this permission notice shall be
//   *  included in all copies or substantial portions of the Software.
//   *
//   *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//   *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//   *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//   *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//   *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//   *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//   *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//   *
//   */
 
// #include <gecode/driver.hh>
// #include <gecode/int.hh>
 
// #include <vector>
// #include <algorithm>
// #include <sstream>
// #include "compact-table.hpp"

//  using namespace Gecode;
 
//  namespace {
//    using std::vector;
 
//    vector<vector<int> > layout;
//    vector<int> layer, pile;
 
//    void generate(int seed) {
//      // The layout consists of 17 piles of 3 cards each
//      layout = vector<vector<int> >(17, vector<int>(3));
//      // Deck without the Ace of Spades
//      vector<int> deck(51);
//      for (int i = 51; i--; ) deck[i] = i+1;
//      Support::RandomGenerator rnd(seed+1);
//      std::random_shuffle(deck.begin(), deck.end(), rnd);
 
//      // Place cards from deck
//      int pos = 0;
//      for (int i = 17; i--; )
//        for (int j = 3; j--; )
//          layout[i][j] = deck[pos++];
 
//      // Location-information for each card
//      layer = vector<int>(52);
//      pile  = vector<int>(52);
//      for (int i = 17; i--; ) {
//        for (int j = 3; j--; ) {
//          layer[layout[i][j]] = j;
//          pile[ layout[i][j]] = i;
//        }
//      }
//    }
//  }
 
//  class BlackHole : public Script {
//  protected:
//    IntVarArray x, 
//      y; 
 
//    std::string
//    card(int val) const {
//      const char* suit = "SCHD";
//      std::ostringstream o;
//      o << std::setw(2) << (1 + (val%13)) << suit[val/13];
//      return o.str();
//    }
 
//  public:
//    enum {
//      SYMMETRY_NONE,       
//      SYMMETRY_CONDITIONAL 
//    };
//    enum {
//      PROPAGATION_REIFIED,  
//      PROPAGATION_DFA,      
//      PROPAGATION_TUPLE_SET 
//    };
//    BlackHole(const SizeOptions& opt)
//      : Script(opt), x(*this, 52, 0,51), y(*this, 52, 0,51) {
//      // Black ace at bottom
//      rel(*this, x[0], IRT_EQ, 0);
 
//      // x is order and y is placement
//      channel(*this, x, y, opt.icl());
 
//      // The placement rules: the absolute value of the difference
//      // between two consecutive cards is 1 or 12.
//      if (opt.propagation() == PROPAGATION_REIFIED) {
//        // Build table for accessing the rank of a card
//        IntArgs modtable(52);
//        for (int i = 0; i < 52; ++i) {
//          modtable[i] = i%13;
//        }
//        for (int i = 0; i < 51; ++i) {
//          IntVar x1(*this, 0, 12), x2(*this, 0, 12);
//          element(*this, modtable, x[i], x1);
//          element(*this, modtable, x[i+1], x2);
//          const int dr[2] = {1, 12};
//          IntVar diff(*this, IntSet(dr, 2));
//          rel(*this, abs(x1-x2) == diff, ICL_DOM);
//        }
//      } else if (opt.propagation() == PROPAGATION_DFA) {
//        // Build table for allowed tuples
//        REG expression;
//        for (int r = 13; r--; ) {
//          for (int s1 = 4; s1--; ) {
//            for (int s2 = 4; s2--; ) {
//              for (int i = -1; i <= 1; i+=2) {
//                REG r1 = REG(r+13*s1);
//                REG r2 = REG((r+i+52+13*s2)%52);
//                REG r = r1 + r2;
//                expression |= r;
//              }
//            }
//          }
//        }
//        DFA table(expression);
 
//        for (int i = 51; i--; ) {
//          extensional(*this, IntVarArgs() << x[i] << x[i+1], table);
//        }
         
 
//      } else { // opt.propagation() == PROPAGATION_TUPLE_SET)
//        // Build table for allowed tuples
//        TupleSet tupleSet;
//        for (int r = 13; r--; )
//          for (int s1 = 4; s1--; )
//            for (int s2 = 4; s2--; )
//              for (int i = -1; i <= 1; i+=2) {
//                tupleSet.add(IntArgs(2, r+13*s1, (r+i+52+13*s2)%52));
//              }
//        tupleSet.finalize();
 
//        for (int i = 51; i--; ) {
         
//          switch (opt.model()) {
//          case 0: {
//            extensional(*this, IntVarArgs() << x[i] << x[i+1], tupleSet);
//            break;
//          }
//          case 1: {
//            extensional2(*this, IntVarArgs() << x[i] << x[i+1], tupleSet);
//            break;
//          }
//          default:
//            break;
//          }
        
//        }
//      }
 
//      // A card must be played before the one under it.
//      for (int i = 17; i--; )
//        for (int j = 2; j--; )
//          rel(*this, y[layout[i][j]] < y[layout[i][j+1]]);
 
//      // Compute and break the conditional symmetries that are dependent
//      // on the current layout.
//      // Two cards with the same rank but different suits are symmetric
//      // with respect to their placement in the black hole if changing
//      // their order does not affect any other card.
//      if (opt.symmetry() == SYMMETRY_CONDITIONAL) {
//        // For all ranks
//        for (int r = 13; r--; ) {
//          // For all pairs of suits
//          for (int s1 = 4; s1--; ) {
//            for (int s2 = s1; s2--; ) {
//              int c1 = 13*s1 + r,
//                c2 = 13*s2 + r;
//              // The ace of spades is already placed
//              if (c1 == 0 || c2 == 0) continue;
//              // Piles are handled by the rules of the game
//              if (pile[c1] == pile[c2]) continue;
//              // Fix the right order of the cards
//              int o1 = c1, o2 = c2;
//              if (pile[c1] > pile[c2] && layer[c2] >= layer[c1])
//                std::swap(o1, o2);
//              // cond is the condition for the symmetry
//              BoolVarArgs ba;
//              // Both cards played after the ones on top of them
//              for (int i = 0; i < layer[o1]; ++i)
//                ba << expr(*this, (y[layout[pile[o1]][i]] < y[o2]));
//              for (int i = 0; i < layer[o2]; ++i)
//                ba << expr(*this, (y[layout[pile[o2]][i]] < y[o1]));
//              // Both cards played before the ones under them
//              for (int i = layer[o1]+1; i < 3; ++i)
//                ba << expr(*this, (y[o2] < y[layout[pile[o1]][i]]));
//              for (int i = layer[o2]+1; i < 3; ++i)
//                ba << expr(*this, (y[o1] < y[layout[pile[o2]][i]]));
//              // Cond holds when all the above holds
//              BoolVar cond(*this, 0, 1);
//              rel(*this, BOT_AND, ba, cond);
 
//              // If cond is fulfilled, then we can order the cards
//              // cond -> (y[o1] < y[o2])
//              rel(*this, !cond || (y[o1] < y[o2]));
//            }
//          }
//        }
//      }
 
//      // Install custom brancher
//      branch(*this, x, INT_VAR_NONE(), INT_VAL(&val));
//    }
//    static int val(const Space&, IntVar x, int) {
//      int v = -1;
//      int w = 4;
//      for (IntVarValues vals(x); vals(); ++vals)
//        if (layer[vals.val()] < w) {
//          v = vals.val();
//          if ((w = layer[vals.val()]) == 0) 
//            break;
//        }
//      assert(v >= 1 && v < 52);
//      return v;
//    }
//    virtual void
//    print(std::ostream& os) const {
//      os << "Layout:" << std::endl;
//      for (int i = 0; i < 17; i++) {
//        for (int j = 0; j < 3; j++)
//          os << card(layout[i][j]) << " ";
//        if ((i+1) % 3 == 0)
//          os << std::endl;
//        else
//          os << "  \t";
//      }
//      os << std::endl << std::endl;
 
//      os << "Solution:" << std::endl;
//      for (int i = 0; i < 52; ++i) {
//        if (x[i].assigned())
//          os << card(x[i].val()) << " ";
//        else
//          os << "   ";
//        if ((i + 1) % 13 == 0)
//          os << std::endl;
//      }
//      os << std::endl;
//      os << std::endl;
//    }
 
//    BlackHole(bool share, BlackHole& s) : Script(share,s) {
//      x.update(*this, share, s.x);
//      y.update(*this, share, s.y);
//    }
//    virtual Space*
//    copy(bool share) {
//      return new BlackHole(share,*this);
//    }
//  };
 
//  int
//  main(int argc, char* argv[]) {
//    SizeOptions opt("Black Hole patience");
//    opt.symmetry(BlackHole::SYMMETRY_CONDITIONAL);
//    opt.symmetry(BlackHole::SYMMETRY_NONE,"none",
//                 "no symmetry breaking");
//    opt.symmetry(BlackHole::SYMMETRY_CONDITIONAL,"conditional",
//                 "break conditional symmetries");
//    opt.propagation(BlackHole::PROPAGATION_TUPLE_SET);
//    opt.propagation(BlackHole::PROPAGATION_REIFIED,
//                    "reified", "use reified propagation");
//    opt.propagation(BlackHole::PROPAGATION_DFA,
//                    "dfa", "use DFA-based extensional propagation");
//    opt.propagation(BlackHole::PROPAGATION_TUPLE_SET,
//                    "tuple-set", "use TupleSet-based extensional propagation");
//    opt.icl(ICL_DOM);
//    opt.model(0, "g", "extensional");
//    opt.model(1, "c", "compact table");
//    opt.parse(argc,argv);
//    // Generates the new board
//    generate(opt.size());
//    Script::run<BlackHole,DFS,SizeOptions>(opt);
//    return 0;
//  }
 
//  // STATISTICS: example-any
