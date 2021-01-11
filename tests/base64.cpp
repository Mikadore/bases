#include <catch2/catch.hpp>
#include <bases.hpp>
#include <cstdint>
#include <array>
//As described in RFC4648
TEST_CASE("Base64 Test Vectors")
{
    using namespace std::string_view_literals;
    REQUIRE(bases::b64encode(""sv)         == ""        );
    REQUIRE(bases::b64encode("f"sv)        == "Zg=="    );
    REQUIRE(bases::b64encode("fo"sv)       == "Zm8="    );
    REQUIRE(bases::b64encode("foo"sv)      == "Zm9v"    );
    REQUIRE(bases::b64encode("foob"sv)     == "Zm9vYg==");
    REQUIRE(bases::b64encode("fooba"sv)    == "Zm9vYmE=");
    REQUIRE(bases::b64encode("foobar"sv)   == "Zm9vYmFy");
}