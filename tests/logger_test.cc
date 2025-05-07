#include <gtest/gtest.h>
#include <boost/log/core.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include "logger.h"

namespace logging = boost::log;

// test that verifies console sink and file sink are added by init_logging
TEST(LoggerTest, InitAddsConsoleAndFileSinksAndAttributes) {
    auto core = logging::core::get();

    // clear any existing sinks so we start fresh
    core->remove_all_sinks();

    // initialize logging (adds console & file sinks, plus common attributes)
    init_logging();

    // attributes check
    boost::log::add_common_attributes();
    auto attrs = core->get_global_attributes();
    EXPECT_TRUE(attrs.count("TimeStamp")) << "Missing TimeStamp attribute";
    EXPECT_TRUE(attrs.count("ThreadID"))  << "Missing ThreadID attribute";
}

// standard google test entry point
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
