#include "AiSettings.h"

#include "ClaudeProvider.h"
#include "OllamaProvider.h"
#include "SecretStore.h"

#include <QSettings>

AiSettings AiSettings::load(const QString &settingsFile)
{
    QSettings ini(settingsFile, QSettings::IniFormat);
    AiSettings settings;
    settings.provider = ini.value("ai/provider").toString() == "claude" ? Provider::Claude : Provider::Ollama;
    settings.ollamaModel = ini.value("ai/ollamaModel", settings.ollamaModel).toString().trimmed();
    settings.claudeModel = ini.value("ai/claudeModel", settings.claudeModel).toString().trimmed();

    const QByteArray encrypted = QByteArray::fromBase64(ini.value("ai/claudeKey").toByteArray());
    if (!encrypted.isEmpty())
        settings.claudeApiKey = SecretStore::decrypt(encrypted).value_or(QString());
    return settings;
}

bool AiSettings::save(const QString &settingsFile) const
{
    QSettings ini(settingsFile, QSettings::IniFormat);
    ini.setValue("ai/provider", provider == Provider::Claude ? "claude" : "ollama");
    ini.setValue("ai/ollamaModel", ollamaModel);
    ini.setValue("ai/claudeModel", claudeModel);

    if (claudeApiKey.isEmpty()) {
        ini.remove("ai/claudeKey");
    } else {
        const std::optional<QByteArray> encrypted = SecretStore::encrypt(claudeApiKey);
        if (!encrypted)
            return false;
        ini.setValue("ai/claudeKey", encrypted->toBase64());
    }
    ini.sync();
    return ini.status() == QSettings::NoError;
}

ChatProvider *createChatProvider(const AiSettings &settings, QObject *parent)
{
    if (settings.provider == AiSettings::Provider::Claude && !settings.claudeApiKey.isEmpty())
        return new ClaudeProvider(settings.claudeApiKey, settings.claudeModel, parent);
    return new OllamaProvider(settings.ollamaModel, parent);
}
