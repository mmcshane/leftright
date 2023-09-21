#include <mpm/leftright.h>
#include <atomic>
#include <map>
#include <catch2/catch.hpp>


TEST_CASE("in-place construct", "[mpm.leftright]")
{
    using lrstring = mpm::leftright<std::string>;

    lrstring lrs(mpm::in_place, 3, '*');
    lrs.observe([](lrstring::const_reference str) {
        CHECK(str == "***");
    });
}


TEST_CASE("modify read modify read", "[mpm.leftright]")
{
    using lrmap = mpm::leftright<std::map<int, int>>;
    lrmap lrm;
    lrm.modify([](lrmap::reference map) noexcept {
        map[1] = 1;
    });

    int value = lrm.observe([](lrmap::const_reference map) { return map.find(1)->second; });
    CHECK(1 == value);


    lrm.modify([](lrmap::reference map) noexcept {
        map[2] = 2;
    });

    value = lrm.observe([](lrmap::const_reference map) { return map.find(2)->second; });
    CHECK(2 == value);
}


TEST_CASE("distributed modify read modify read", "[mpm.leftright]")
{
    using lrmap = mpm::basic_leftright<std::map<int, int>,
          mpm::distributed_atomic_reader_registry<4>>;
    lrmap lrm;
    lrm.modify([](lrmap::reference map) noexcept {
        map[1] = 1;
    });

    int value = lrm.observe([](lrmap::const_reference map) { return map.find(1)->second; });
    CHECK(1 == value);


    lrm.modify([](lrmap::reference map) noexcept {
        map[2] = 2;
    });

    value = lrm.observe([](lrmap::const_reference map) { return map.find(2)->second; });
    CHECK(2 == value);
}


TEST_CASE("noexcept specs", "[mpm.leftright]")
{
    using lrint = mpm::leftright<int>;
    lrint lri{0};

    auto null_observer = [](lrint::const_reference i){ return i; };
    auto noexcept_null_observer = [](lrint::const_reference i) noexcept { return i; };

    CHECK(noexcept(lri.observe(noexcept_null_observer)));
    CHECK(!noexcept(lri.observe(null_observer)));

    auto noexcept_null_modifier = [](lrint::reference i) noexcept { return i; };

    CHECK(0 == lri.modify(noexcept_null_modifier));

    // this doesn't and shouldn't compile because modify only exists for
    // functors that are declared noexcept
    //auto null_modifier = [](lrint::reference i){ return i; };
    //CHECK(0 == lri.modify(null_modifier));
}
TEST_CASE("hammer it", "[mpm.leftright]")
{
    // repro of https://github.com/mmcshane/leftright/issues/3

    // this test may require compiler optimizations (e.g. -O3) to fail as
    // expected. To induce failure, change the memory order used in the stores
    // to basic_leftright::m_leftright performed in the two branches of
    // basic_leftright::modify to std::memory_order_release.

    struct Bigish {
      public:
        void add1() { a1 += 1; a2 += 1; a3 += 1; a4 += 1; }

        alignas(MPM_LEFTRIGHT_CACHE_LINE_SIZE) int a1 = 0;
        alignas(MPM_LEFTRIGHT_CACHE_LINE_SIZE) int a2 = 0;
        alignas(MPM_LEFTRIGHT_CACHE_LINE_SIZE) int a3 = 0;
        alignas(MPM_LEFTRIGHT_CACHE_LINE_SIZE) int a4 = 0;
    };

    mpm::leftright<Bigish> lrb{};

    std::atomic_uint_fast32_t violations{0};
    std::thread reader([&] {
        for(int i = 0; i < 10000000; i++) {
            auto observed = lrb.observe([](const Bigish& val) noexcept{ return val; });
            auto a1 = observed.a1;
            if (observed.a2 != a1 || observed.a3 != a1 || observed.a4 != a1) {
                violations.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    Bigish in{};
    for(int i = 0; i < 10000000; i++){
        in.add1();
        lrb.modify([&](Bigish& b)noexcept{ b = in; });
    }

    reader.join();

    CHECK(0 == violations.load(std::memory_order_relaxed));
}

/*
struct unsafe_reader_registry
{
    void arrive(){}; //missing required noexcept
    void depart(){}; //missing required noexcept
    bool empty() const { return true; } //missing required noexcept
};

// This shouldn't and doesn't compile because the functions on
// unsafe_reader_registry are not noexcept
template class mpm::basic_leftright<int, unsafe_reader_registry>;
*/
