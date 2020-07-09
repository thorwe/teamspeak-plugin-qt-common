#include "core/ts_error.h"

#include "core/ts_functions.h"

namespace com::teamspeak
{
ts_errc to_ts_errc(uint32_t err)
{
    return static_cast<ts_errc>(err);
}
}  // namespace com::teamspeak

namespace
{

struct TsErrCategory : std::error_category
{
    [[nodiscard]] const char *name() const noexcept override;
    [[nodiscard]] std::string message(int ev) const override;
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
