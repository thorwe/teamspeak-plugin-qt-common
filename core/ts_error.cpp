#include "core/ts_error.h"

#include "core/ts_functions.h"

namespace
{  // anonymous namespace

struct TsErrCategory : std::error_category
{
    const char *name() const noexcept override;
    std::string message(int ev) const override;
};

const char *TsErrCategory::name() const noexcept
{
    return "TeamSpeak";
}

std::string TsErrCategory::message(int ev) const
{
    return com::teamspeak::pluginsdk::funcs::get_error_message(ev);
}

const TsErrCategory theTsErrCategory{};

}  // anonymous namespace

std::error_code make_error_code(ts_errc e)
{
    return {static_cast<int>(e), theTsErrCategory};
}
