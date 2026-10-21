#pragma once

#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>

namespace con::log
{
    enum class Category : uint32_t
    {
        Boot = 1u << 0,
        Loader = 1u << 1,
        File = 1u << 2,
        Ee = 1u << 3,
        Iop = 1u << 4,
        Gs = 1u << 5,
        Vu = 1u << 6,
        Spu2 = 1u << 7,
        Input = 1u << 8,
        Game = 1u << 9,
        Hook = 1u << 10,
    };

    enum class Level
    {
        Error,
        Warn,
        Info,
        Trace,
    };

    void configureFromEnvironment();
    void setTraceMask(uint32_t mask);
    bool traceEnabled(Category category);
    void write(Category category, Level level, std::string_view message);
    // Seconds since the log started: the time stamp of every log line ("[  12.3456]").
    double elapsedSeconds();
    std::string_view name(Category category);
}

#define CON_LOG(cat, lvl, expr)                                                   \
    do                                                                             \
    {                                                                              \
        if ((lvl) != ::con::log::Level::Trace || ::con::log::traceEnabled(cat)) \
        {                                                                          \
            std::ostringstream conLogStream_;                                     \
            conLogStream_ << expr;                                                \
            ::con::log::write((cat), (lvl), conLogStream_.str());                \
        }                                                                          \
    } while (0)

#define CON_INFO(cat, expr) CON_LOG(::con::log::Category::cat, ::con::log::Level::Info, expr)
#define CON_WARN(cat, expr) CON_LOG(::con::log::Category::cat, ::con::log::Level::Warn, expr)
#define CON_ERROR(cat, expr) CON_LOG(::con::log::Category::cat, ::con::log::Level::Error, expr)
#define CON_TRACE(cat, expr) CON_LOG(::con::log::Category::cat, ::con::log::Level::Trace, expr)
