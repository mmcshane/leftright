#include <mpm/leftright.h>
#include <map>
#include <string>
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
