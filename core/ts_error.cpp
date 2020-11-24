#include "core/ts_error.h"

#include "core/ts_functions.h"

namespace com::teamspeak
{
auto to_ts_errc(uint32_t err) -> ts_errc
{
    return static_cast<ts_errc>(err);
}
}  // namespace com::teamspeak

namespace
{

struct TsErrCategory : std::error_category
{
    [[nodiscard]] auto name() const noexcept -> const char * override;
    [[nodiscard]] auto message(int ev) const -> std::string override;
};

auto TsErrCategory::name() const noexcept -> const char *
{
    return "TeamSpeak";
}

auto TsErrCategory::message(int ev) const -> std::string
{
    return com::teamspeak::pluginsdk::funcs::get_error_message(ev);
}

const TsErrCategory theTsErrCategory{};

}  // anonymous namespace

auto make_error_code(ts_errc e) -> std::error_code
{
    return {static_cast<int>(e), theTsErrCategory};
}
