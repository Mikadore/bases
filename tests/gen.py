# tests/gen.py
# (c) Mikdore 2021
# This file is licensed under the Boost Software License
#
import base64, random

def encode_case(bytes, base):
    s = "\t{\n\t\tstd::uint8_t TEST_DATA[] = {"

    delim = ''
    for byte in bytes:
        s += (delim + "0x{:X}".format(byte))
        delim = ','

    s += f"}};\n\t\tREQUIRE(bases::converter<bases::base::BASE{base}>::encode((std::uint8_t*) TEST_DATA, {len(bytes)}) == \""
    
    basestr = base64.b64encode(bytes).decode('ascii')
    
    s +=  f"{basestr}\");\n\t\tauto bytes = bases::converter<bases::base::BASE{base}>::decode(\"{basestr}\", {len(basestr)});\n\t\t"
    s += f"REQUIRE(std::equal(std::begin(TEST_DATA), std::end(TEST_DATA), std::begin(*bytes)));\n\t}}\n"

    return s


if __name__ == "__main__":
    for i in [64]: #, 32, 16]:
        with open(f"tests/generated/base{i}-fuzz.cpp", "w") as file:
            file.write(
                "/**"
            +   " * (c) Mikdore 2021 \n"
            +   " * The content of this file has been automatically generated and is licensed under the Boost Software License\n"
            +   " */\n"
            +   "#include <bases/bases.hpp>\n#include <cstdint>\n"
            +   "#include <catch2/catch.hpp>\nTEST_CASE(\"Base64 encoding\")\n{\n"
            )
            for j in range(1, 512):
                for k in range(4):
                    byteArr = bytearray(random.getrandbits(8) for _ in range(j))
                    file.write(encode_case(byteArr, i))
            file.write("}\n")

