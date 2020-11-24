#include "core/translator.h"

#include "core/ts_helpers_qt.h"
#include "core/ts_logging_qt.h"

#include <QtCore/QString>

#include <utility>

Translator::Translator(QObject* parent, QString prefix)
	: QTranslator(parent)
    , kPrefix(std::move(prefix))
{}

auto Translator::update() -> bool
{
    const auto lang = TSHelpers::GetLanguage();
    if (lang.isEmpty())
        return false;

    const auto is_translate = load(QStringLiteral(":/locales/") + kPrefix + "_" + lang);
    if (!is_translate)
        TSLogging::Log("No translation available.");

    return is_translate;
}
