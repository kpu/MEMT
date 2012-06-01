#include "MEMT/Input/Transitive.hh"

#include "MEMT/Input/Input.hh"
#include "MEMT/Input/Location.hh"

#include <assert.h>
#include <set>
#include <stack>

using namespace std;

namespace input {

void CloseAlignment(const Input &text, const Location &start, set<Location> &seen) {
  // Words left to investigate.
  stack<Location> open;
  open.push(start);
  seen.insert(start);
  while (!open.empty()) {
    Location pos = open.top();
    open.pop();
    assert(pos.engine < text.engines.size());
    assert(pos.offset < text.engines[pos.engine].words.size());
    const WordAlignments &alignments = text.GetWord(pos).alignments;
    for (unsigned int other = 0; other < text.engines.size(); ++other) {
      if (alignments.Ask(other).IsConnection()) {
        Location to_ins(other, alignments.Ask(other).offset);
        if (seen.insert(to_ins).second) open.push(to_ins);
      }
    }
  }
}

bool MakeAlignmentsTransitive(Input &text) {
  vector<set<unsigned int> > unused(text.engines.size());
  for (unsigned int e = 0; e < text.engines.size(); ++e) {
    const Engine &engine = text.engines[e];
    for (unsigned offset = 0; offset < engine.words.size(); ++offset) {
      unused[e].insert(unused[e].end(), offset);
    }
  }
  bool any_conflict = false;
  for (unsigned int e = 0; e < unused.size(); ++e) {
    set<unsigned int> &offsets = unused[e];
    while (!offsets.empty()) {
      Location start(e, *offsets.begin());
      // All words we've seen.
      set<Location> closure;
      CloseAlignment(text, start, closure);
      vector<unsigned int> count(text.engines.size());
      // Use the entire transitive closure.
      for (set<Location>::const_iterator i = closure.begin(); i != closure.end(); ++i) {
        unused[i->engine].erase(i->offset);
        count[i->engine]++;
      }
      bool conflict = false;
      for (vector<unsigned int>::const_iterator i = count.begin(); i != count.end(); ++i) {
        if (*i > 1) {
          conflict = true;
          break;
        }
      }
      if (conflict) {
      //  cerr << "Transitive conflict from " << start.engine << " word " << start.offset << endl;
        any_conflict = true;
        continue;
      }
      /* No conflict.  Tell all the alignments.
       * It would be more efficient for the alignments to just share a common data
       * structure, but that would require transitivity everywhere.
       */
      for (set<Location>::const_iterator t = closure.begin(); t != closure.end(); ++t) {
        WordAlignments &telling = text.engines[t->engine].words[t->offset].alignments;
        for (set<Location>::const_iterator about = closure.begin(); about != closure.end(); ++about) {
          telling.Add(about->engine, about->offset, AL_TRANSITIVE);
        }
        
      }
    }
  }
  return true;
}

}  // namespace input
