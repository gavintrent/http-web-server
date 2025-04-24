#include "logger.h"
// dependencies for writing and formatting logs, attaching attributes
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes/current_thread_id.hpp>

// create namespaces for simpler syntax
namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

//
void init_logging() {
    // set up log sink that writes to terminal
    logging::add_console_log(std::clog,
        logging::keywords::format = (
            expr::stream
                << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
                << "] [" << logging::trivial::severity
                << "] [Thread " << expr::attr<attrs::current_thread_id::value_type>("ThreadID")
                << "] " << expr::smessage
        )); // format: [2025-04-24 12:00:00] [info] [Thread 0x123456] your message here

    logging::add_file_log( // write to log file, create new file at midnight or 10MB
        logging::keywords::file_name = "../logs/server_%Y-%m-%d.log",
        logging::keywords::rotation_size = 10 * 1024 * 1024, // 10 MB
        logging::keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        logging::keywords::auto_flush = true,
        logging::keywords::format = (
            expr::stream
                << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
                << "] [" << logging::trivial::severity
                << "] [Thread " << expr::attr<attrs::current_thread_id::value_type>("ThreadID")
                << "] " << expr::smessage
        ) // format: [2025-04-24 12:00:00] [info] [Thread 0x123456] your message here
    );

    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::trace
    );

    logging::add_common_attributes(); // adds TimeStamp, ThreadID
}