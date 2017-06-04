#pragma once

#include <QtCore/QTranslator>

class Translator : public QTranslator
{
	Q_OBJECT

public:
	Translator(QObject *parent = nullptr, QString prefix = "plugin");
    bool update();

private:
	const QString kPrefix;
};
