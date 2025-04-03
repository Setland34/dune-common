// SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
// SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#include <cstdint>
#include <new>
#include <tuple>
#include <utility>

#include <dune/common/debugalign.hh>
#include <dune/common/hybridutilities.hh>
#include <dune/common/parallel/mpihelper.hh>
#include <dune/common/test/arithmetictestsuite.hh>
#include <dune/common/test/testsuite.hh>

//! Exception thrown when an object is allocated with a misaligned address
class MisalignedAddress : public Dune::RangeError {};

class WithViolatedAlignmentHandler {
  Dune::ViolatedAlignmentHandler oldhandler;
public:
  template<class H>
  WithViolatedAlignmentHandler(H &&newhandler) :
    oldhandler(Dune::violatedAlignmentHandler())
  {
    Dune::violatedAlignmentHandler() = std::forward<H>(newhandler);
  }

  WithViolatedAlignmentHandler(const WithViolatedAlignmentHandler &) = delete;
  WithViolatedAlignmentHandler(WithViolatedAlignmentHandler &&) = delete;

  WithViolatedAlignmentHandler&
  operator=(const WithViolatedAlignmentHandler &) = delete;
  WithViolatedAlignmentHandler&
  operator=(WithViolatedAlignmentHandler &&) = delete;

  ~WithViolatedAlignmentHandler()
  {
    Dune::violatedAlignmentHandler() = oldhandler;
  }
};

// intentionally violate alignment and check that that is detected
template<class T>
void checkAlignmentViolation(Dune::TestSuite &test)
{
  WithViolatedAlignmentHandler
    guard([&](auto&&...){ throw MisalignedAddress{}; });

  static_assert(alignof(T) <= sizeof(T));
  char buffer[alignof(T)+sizeof(T)];

  void* misalignedAddr = std::addressof(buffer);
  {
    // take any address, align it to the correct value...
    std::size_t space = alignof(T)+sizeof(T);
    misalignedAddr = std::align(alignof(T), sizeof(T), misalignedAddr, space);
    // and move one past its alignmed address to make sure it's misalingned
    misalignedAddr = static_cast<char*>(misalignedAddr) + 1;
    test.check(not Dune::isAligned(misalignedAddr, alignof(T)), "We could not misalign an address");
  }
auto ptr = new(misalignedAddr) T;
test.check(misalignmentDetected, "default construct")
  << "misalignment not detected for " << Dune::className<T>();

misalignmentDetected = false;

ptr = new(misalignedAddr) T(T(0));
test.check(misalignmentDetected, "move construct")
  << "misalignment not detected for " << Dune::className<T>();
ptr->~T();

misalignmentDetected = false;

T t(0);
ptr = new(misalignedAddr) T(t);
test.check(misalignmentDetected, "copy construct")
  << "misalignment not detected for " << Dune::className<T>();
ptr->~T();
=======
  test.checkThrow<MisalignedAddress>([&]{
    std::ignore = new(misalignedAddr) T;
  }, "default construct") << "misaligned address was not caught" << Dune::className<T>();

  test.checkThrow<MisalignedAddress>([&]{
    std::ignore = new(misalignedAddr) T(T(0));
  }, "move construct") << "misaligned address was not caught";

  T t(0);
  test.checkThrow<MisalignedAddress>([&]{
    std::ignore = new(misalignedAddr) T(t);
  }, "copy construct") << "misaligned address was not caught";
auto ptr = new(misalignedAddr) T;
test.check(misalignmentDetected, "default construct")
  << "misalignment not detected for " << Dune::className<T>();

misalignmentDetected = false;

ptr = new(misalignedAddr) T(T(0));
test.check(misalignmentDetected, "move construct")
  << "misalignment not detected for " << Dune::className<T>();
ptr->~T();

misalignmentDetected = false;

T t(0);
ptr = new(misalignedAddr) T(t);
test.check(misalignmentDetected, "copy construct")
  << "misalignment not detected for " << Dune::className<T>();
ptr->~T();