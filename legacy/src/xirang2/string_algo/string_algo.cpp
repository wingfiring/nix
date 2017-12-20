#include <xirang2/string_algo/string.h>

#include <array>
#include <cctype>
namespace xirang2 {namespace str_algo {
    namespace {
        typedef unsigned char uchar_type;
        std::array<unsigned char, 256> make_icase_table() {
            std::array<unsigned char, 256> ret;
            for (int i = 0; i < int(ret.size()); ++i) {
                if (std::isupper(i)) {
                    ret[i] = uchar_type(std::tolower(i));
                }
                else
                    ret[i] = uchar_type(i);
            }
            return ret;
        }
        const std::array<unsigned char, 256> table = make_icase_table();
    }

    bool icase_compare::operator()(char lhs, char rhs) const {
        return table[uchar_type(lhs)] == table[uchar_type(rhs)];
    }
} }
