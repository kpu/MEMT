#include "util/n_best.hh"

#define BOOST_TEST_MODULE NBestTest
#include <boost/test/unit_test.hpp>

namespace nbest {
namespace {

BOOST_AUTO_TEST_CASE(is_not_available) {
  unsigned int values[10];
  detail::IsNotAvailable<unsigned int> is_not4(values[4]);
  BOOST_CHECK_EQUAL(true, is_not4(values[0]));
  BOOST_CHECK_EQUAL(true, is_not4(values[10]));
  BOOST_CHECK_EQUAL(false, is_not4(values[4]));
}

BOOST_AUTO_TEST_CASE(array_storage) {
  typedef detail::ArrayStorage<int> Storage;
  Storage storage;
  storage.Reset(3);
  BOOST_CHECK_EQUAL((size_t)0, storage.size());
  BOOST_CHECK_EQUAL((size_t)3, storage.max_size());
  BOOST_CHECK(storage.begin() == storage.end());

  storage.Available() = 1; storage.UseAvailable();
  BOOST_REQUIRE_EQUAL((size_t)1, storage.size());
  Storage::iterator i = storage.begin(); 
  BOOST_CHECK_EQUAL(1, *(i++));
  BOOST_CHECK(storage.end() == i);

  storage.Available() = 2; storage.UseAvailable();
  BOOST_REQUIRE_EQUAL((size_t)2, storage.size());
  i = storage.begin();
  BOOST_CHECK_EQUAL(1, *(i++));
  BOOST_CHECK_EQUAL(2, *(i++));
  BOOST_CHECK(storage.end() == i);

  storage.Available() = 3; storage.UseAvailable();
  BOOST_REQUIRE_EQUAL((size_t)3, storage.size());
  i = storage.begin();
  BOOST_CHECK_EQUAL(1, *(i++));
  BOOST_CHECK_EQUAL(2, *(i++));
  BOOST_CHECK_EQUAL(3, *(i++));
  BOOST_CHECK(storage.end() == i);

  storage.Available() = 10; storage.ReplaceAvailable(*storage.begin());
  BOOST_REQUIRE_EQUAL((size_t)3, storage.size());
  i = storage.begin();
  BOOST_CHECK_EQUAL(2, *(i++));
  BOOST_CHECK_EQUAL(3, *(i++));
  BOOST_CHECK_EQUAL(10, *(i++));
  BOOST_CHECK(storage.end() == i);
}

/*void DumpSetDropper(const detail::SetDropper<int, std::less<int> > &dropper) {
  for (detail::SetDropper<int, std::less<int> >::const_iterator i = dropper.begin(); i != dropper.end(); ++i) {
    std::cerr << **i << " at " << *i << std::endl;
  }
}*/

template <class Dropper> void TestDropper() {
  typedef detail::ArrayStorage<int> Storage;
  Storage storage;
  storage.Reset(3);

  Dropper dropper(storage, typename Dropper::Less());
  storage.Available() = 1; dropper.Insert(storage.Available()); storage.UseAvailable();
  BOOST_CHECK_EQUAL(1, dropper.PeekBottom());
  storage.Available() = -2; dropper.Insert(storage.Available()); storage.UseAvailable();
  BOOST_CHECK_EQUAL(-2, dropper.PeekBottom());
  storage.Available() = 3; dropper.Insert(storage.Available()); storage.UseAvailable();
  BOOST_CHECK_EQUAL(-2, dropper.PeekBottom());

  int &bottom = dropper.PopBottom();
  BOOST_CHECK_EQUAL(-2, bottom);
  BOOST_CHECK_EQUAL(1, dropper.PeekBottom());

  storage.Available() = 7; dropper.Insert(storage.Available()); storage.ReplaceAvailable(bottom);
  BOOST_CHECK_EQUAL(1, dropper.PeekBottom());
  storage.Available() = 3; dropper.Insert(storage.Available()); storage.ReplaceAvailable(dropper.PopBottom());

  BOOST_CHECK_EQUAL(3, dropper.PeekBottom());
  Storage::iterator st = storage.begin();
  BOOST_CHECK_EQUAL(3, *st);
  BOOST_CHECK_EQUAL(3, *(++st));
  BOOST_CHECK_EQUAL(7, *(++st));
  BOOST_CHECK(++st == storage.end());

  typedef typename Dropper::Update Update;
  {
    st = storage.begin();
    Update update(dropper, *st);
    *st += 5;
    BOOST_CHECK_EQUAL(8, *st);
    BOOST_CHECK_EQUAL(3, *(++st));
    BOOST_CHECK_EQUAL(7, *(++st));
    update.Commit(true);
    BOOST_REQUIRE_EQUAL(3, dropper.PeekBottom());
  }

  // Now contains 8, 3, 7.
  for (typename Storage::iterator i = storage.begin(); i != storage.end(); ++i) {
    Update update(dropper, *i);
    *i += 5;
    update.Commit(true);
  }
  BOOST_CHECK_EQUAL(8, dropper.PeekBottom());
  st = storage.begin();
  BOOST_CHECK_EQUAL(13, *st);
  BOOST_CHECK_EQUAL(8, *(++st));
  BOOST_CHECK_EQUAL(12, *(++st));
  BOOST_CHECK(++st == storage.end());

  BOOST_CHECK_EQUAL(8, dropper.PeekBottom());
  for (typename Storage::iterator i = storage.begin(); i != storage.end(); ++i) {
    Update update(dropper, *i);
    *i -= 12;
    update.Commit(true);
  }
  BOOST_CHECK_EQUAL(-4, dropper.PeekBottom());
}

BOOST_AUTO_TEST_CASE(heap_dropper) {
  TestDropper<detail::HeapDropper<int, std::less<int> > >();
}

BOOST_AUTO_TEST_CASE(set_dropper) {
  TestDropper<detail::SetDropper<int, std::less<int> > >();
}

BOOST_AUTO_TEST_CASE(n_best) {
  typedef NBest<int, std::less<int>, HashDupe<int>, NullMerge<int> > List;

  List n_best;
  n_best.Reset(5);
  BOOST_CHECK_EQUAL((size_t)5, n_best.max_size());
  BOOST_CHECK_EQUAL((size_t)0, n_best.size());
  BOOST_CHECK_EQUAL(true, n_best.empty());

  BOOST_CHECK_EQUAL(true, n_best.MayMakeIt(1));
  n_best.Available() = 1;
  n_best.InsertAvailable();
  BOOST_CHECK_EQUAL((size_t)1, n_best.size());
  BOOST_CHECK_EQUAL(false, n_best.empty());
  {
    List::unordered_iterator i = n_best.unordered_begin();
    BOOST_CHECK_EQUAL(1, *i);
    BOOST_CHECK(++i == n_best.unordered_end());
  }

  // Basic dupe elimination.
  n_best.Available() = 1;
  n_best.InsertAvailable();
  BOOST_CHECK_EQUAL((size_t)1, n_best.size());
  {
    List::unordered_iterator i = n_best.unordered_begin();
    BOOST_CHECK_EQUAL(1, *i);
    BOOST_CHECK(++i == n_best.unordered_end());
  }

  const int add[10] = {2, 7, 1, 3, 2, 0, -1, 16, -4, 3};
  for (const int *i = &add[0]; i != &add[9]; ++i) {
    n_best.Available() = *i;
    n_best.InsertAvailable();
  }
  BOOST_CHECK_EQUAL((size_t)5, n_best.size());
  n_best.destructive_ordered_make();
  {
    List::decreasing_iterator i = n_best.destructive_decreasing_begin();
    BOOST_CHECK_EQUAL(16, *i);
    BOOST_CHECK_EQUAL(7, *(++i));
    BOOST_CHECK_EQUAL(3, *(++i));
    BOOST_CHECK_EQUAL(2, *(++i));
    BOOST_CHECK_EQUAL(1, *(++i));
    BOOST_CHECK(++i == n_best.destructive_decreasing_end());
  }
}

/* Unfortunately relaxed_heap only allows update to decrease score.
 *BOOST_AUTO_TEST_CASE(relaxed_heap) {
 *  int storage[4];
 *  boost::relaxed_heap<int*, boost::indirect_fun<std::less<int> >, detail::SubtractPropertyMap<int> > heap(4, boost::indirect_fun<std::less<int> >(), detail::SubtractPropertyMap<int>(storage));
 *  storage[2] = 3;
 *  heap.push(&storage[2]);
 *  storage[3] = 7;
 *  heap.push(&storage[3]);
 *  storage[1] = 3;
 *  heap.push(&storage[1]);
 *  BOOST_CHECK_EQUAL(&storage[1] , heap.top());
 *  BOOST_CHECK_EQUAL(3, *heap.top());
 *  storage[1] = 8;
 *  heap.update(&storage[1]);
 *  BOOST_CHECK_EQUAL(&storage[2], heap.top());
 *  BOOST_CHECK_EQUAL(3, *heap.top());
 *}
 */

} // namespace
} // namespace nbest
