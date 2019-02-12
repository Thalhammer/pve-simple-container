#pragma once
#include <string>
#include <iostream>
#include <stdexcept>
#include "string_helper.h"

namespace pvesc {
	namespace common {
		class version {
            static int compare_section(const std::string& as, const std::string& bs) {
                auto a = as.data();
                auto aend = a + as.size();
                auto b = bs.data();
                auto bend = b + bs.size();
                while (a < aend && b < bend) {
                    while (a < aend && b < bend && !isdigit(*a) && !isdigit(*b)) {
                        if (*a != *b) {
                            std::cout << "1 " << a << " " << b << std::endl;
                            if (*a == '~') return -1;
                            if (*b == '~') return 1;
                            return *a < *b ? -1 : 1;
                        }
                        a++;
                        b++;
                    }
                    if (a < aend && b < bend && (!isdigit(*a) || !isdigit(*b))) {
                        std::cout << "2 " << a << " " << b << std::endl;
                        if (*a == '~') return -1;
                        if (*b == '~') return 1;
                        return isdigit(*a) ? -1 : 1;
                    }
                
                    char *next_a, *next_b;
                    long int num_a = strtol(a, &next_a, 10);
                    long int num_b = strtol(b, &next_b, 10);
                    if (num_a != num_b) {
                        return num_a < num_b ? -1 : 1;
                    }
                    a = next_a;
                    b = next_b;
                }
                if (a >= aend && b >= bend) {
                    return 0;
                } else if (a < aend) {
                    return *a == '~' ? -1 : 1;
                } else {
                    return *b == '~' ? 1 : -1;
                }
            }
        public:
            size_t epoch;
            std::string upstream;
            std::string revision;
            version(const std::string& v) {
                auto col = v.find(':');
                if(col != std::string::npos) {
                    epoch = std::stoul(v.substr(0, col));
                    col++; // Ignore :
                } else {
                    epoch = 0;
                    col = 0;
                }
                auto dash = v.rfind('-');
                if(dash != std::string::npos) {
                    upstream = v.substr(col, dash-col);
                    revision = v.substr(dash + 1);
                } else {
                    upstream = v.substr(col);
                    revision = "0";
                }
                if(upstream.empty() || revision.empty())
                    throw std::invalid_argument("invalid version " + v);
            }

            version()
                : epoch(0), upstream("0"), revision("")
            {}

            /**
             * -1 = this < other
             * 0  = this == other
             * 1  = this > other
             */
            int compare(const version& other) const noexcept {
                if(epoch > other.epoch) return 1;
                else if(epoch < other.epoch) return -1;

                int res = compare_section(upstream, other.upstream);
                if(res != 0) return res;
                return compare_section(revision, other.revision);
            }

            std::string to_string() const {
                std::string res;
                if(epoch != 0) res = std::to_string(epoch) + ":";
                res += upstream;
                if(revision.empty())
                    res += "-" + revision;
                return res;
            }
        };

        struct version_check {
            enum operation {
                gt,
                lt,
                gteq,
                lteq,
                eq,
                ne,
                any
            };
            operation op;
            version with;

            bool operator()(const common::version& ver) const {
                switch(op) {
                    case version_check::any: return true;
                    case version_check::eq: return ver.compare(with) == 0;
                    case version_check::ne: return ver.compare(with) != 0;
                    case version_check::lteq: return ver.compare(with) <= 0;
                    case version_check::gteq: return ver.compare(with) >= 0;
                    case version_check::lt: return ver.compare(with) < 0;
                    case version_check::gt: return ver.compare(with) > 0;
                    default: throw std::logic_error("Invalid operator value");
                }
            }

            std::string to_string() const {
                std::string res;
                switch(op) {
                    case version_check::eq: res = "="; break;
                    case version_check::ne: res = "!="; break;
                    case version_check::lteq: res = "<="; break;
                    case version_check::gteq: res = ">="; break;
                    case version_check::lt: res = "<"; break;
                    case version_check::gt: res = ">"; break;
                    case version_check::any: return "";
                    default: throw std::logic_error("Invalid operator value");
                }
                res += with.to_string();
                return res;
            }

            static version_check parse(const std::string& str) {
                if(str.find("=") == 0) return version_check{eq, version(trim_copy(str.substr(1)))};
                else if(str.find("!=") == 0) return version_check{ne, version(trim_copy(str.substr(2)))};
                else if(str.find("<=") == 0) return version_check{lteq, version(trim_copy(str.substr(2)))};
                else if(str.find(">=") == 0) return version_check{gteq, version(trim_copy(str.substr(2)))};
                else if(str.find("<") == 0) return version_check{lt, version(trim_copy(str.substr(1)))};
                else if(str.find(">") == 0) return version_check{gt, version(trim_copy(str.substr(1)))};
                else return version_check{any, version("0")};
            }
        };

        inline bool operator==(const version& a, const version& b) {
            return a.compare(b) == 0;
        }
        inline bool operator!=(const version& a, const version& b) {
            return a.compare(b) != 0;
        }
        inline bool operator<(const version& a, const version& b) {
            return a.compare(b) < 0;
        }
        inline bool operator>(const version& a, const version& b) {
            return a.compare(b) > 0;
        }
        inline bool operator<=(const version& a, const version& b) {
            return a.compare(b) <= 0;
        }
        inline bool operator>=(const version& a, const version& b) {
            return a.compare(b) >= 0;
        }
	}
}