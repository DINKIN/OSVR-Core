/** @file
    @brief Header

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2016 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_LineLogger_h_GUID_743865A8_E989_4A87_82D9_9BBF3E4C199D
#define INCLUDED_LineLogger_h_GUID_743865A8_E989_4A87_82D9_9BBF3E4C199D

// Internal Includes
#include <osvr/Util/Export.h>
#include <osvr/Util/LogLevel.h>

// Library/third-party includes
// - none

// Standard includes
#include <memory>  // for std::unique_ptr
#include <sstream> // for std::ostringstream
#include <string>  // for std::string
#include <utility> // for std::forward

// Forward declaration
namespace spdlog {
namespace details {
    struct log_msg;
    class line_logger;
} // namespace details
} // namespace spdlog

namespace osvr {
namespace util {
    namespace log {

        // Forward declaration
        class Logger;

        namespace detail {

            /**
             * @brief Handles the mechanics of a single log line/message -
             * implemented as a wrapper class for spdlog::details::line_logger.
             */
            class LineLogger {
              public:
                /**
                 * @brief Move-only.
                 */
                //@{
                OSVR_UTIL_EXPORT LineLogger(LineLogger &&other);
                OSVR_UTIL_EXPORT LineLogger(const LineLogger &other) = delete;
                OSVR_UTIL_EXPORT LineLogger &
                operator=(const LineLogger &) = delete;
                OSVR_UTIL_EXPORT LineLogger &operator=(LineLogger &&) = delete;
                //@}

                /**
                 * @brief Log the message using the callback logger.
                 */
                OSVR_UTIL_EXPORT ~LineLogger();

                /// Outputs the given string.
                OSVR_UTIL_EXPORT void write(const char *what);

                /// Outputs the given string - usually composed by a
                /// StreamProxy.
                OSVR_UTIL_EXPORT void write(const std::string &what);

                /// An object returned by operator<< on a LineLogger, serves to
                /// accumulate streamed output in a single ostringstream then
                /// write it to the linelogger at the end of the expression's
                /// lifetime.
                class StreamProxy {
                  public:
                    StreamProxy(LineLogger &lineLogger)
                        : lineLogger_(lineLogger), os_(new std::ostringstream) {
                    }

                    /// destructor appends the finished stringstream at the end
                    /// of the expression.
                    ~StreamProxy() {
                        if (active_ && os_) {
                            lineLogger_.write(os_->str());
                        }
                    }

                    /// move construction
                    StreamProxy(StreamProxy &&other)
                        : lineLogger_(other.lineLogger_),
                          os_(std::move(other.os_)), active_(other.active_) {
                        other.active_ = false;
                    }

                    StreamProxy(StreamProxy const &) = delete;
                    StreamProxy &operator=(StreamProxy const &) = delete;

                    operator std::ostream &() { return (*os_); }

                    template <typename T> std::ostream &operator<<(T &&what) {
                        (*os_) << std::forward<T>(what);
                        return (*os_);
                    }

                  private:
                    LineLogger &lineLogger_;
                    std::unique_ptr<std::ostringstream> os_;
                    bool active_ = true;
                };

                template <typename T> StreamProxy operator<<(T &&what) {
                    StreamProxy proxy(*this);
                    proxy << std::forward<T>(what);
                    return proxy;
                }

                OSVR_UTIL_EXPORT void disable();
                OSVR_UTIL_EXPORT bool is_enabled() const;

              private:
                /// Construct from rvalue-reference to a spdlog (implementation)
                /// line logger.
                OSVR_UTIL_EXPORT
                LineLogger(spdlog::details::line_logger &&line_logger);

                /// Construct fallback line-logger (when spdlog wasn't
                /// available) from logger name and level.
                OSVR_UTIL_EXPORT
                LineLogger(std::string const &name, LogLevel level);

                /// Construct fallback line-logger (when spdlog wasn't
                /// available) from logger name and level, and immediately start
                /// with a message.
                OSVR_UTIL_EXPORT
                LineLogger(std::string const &name, LogLevel level,
                           const char *msg);

                /// So logger may construct line-loggers. Nobody else needs to,
                /// and assumptions may be violated if others do.
                friend class ::osvr::util::log::Logger;

                bool fallbackEnabled() const { return name_ != nullptr; }
                std::ostream &startFallbackMessaging();
                std::ostream &getFallbackStream() const;
                std::unique_ptr<spdlog::details::line_logger> lineLogger_;
                /// @name Members used in case of fallback operation: null
                /// spdlog lineLogger
                /// @{
                std::string const *name_ = nullptr;
                LogLevel level_ = LogLevel::trace;
                bool startedFallback_ = false;
                /// @}
            };

        } // namespace detail

    } // namespace log

} // namespace util
} // namespace osvr

#endif // INCLUDED_LineLogger_h_GUID_743865A8_E989_4A87_82D9_9BBF3E4C199D
