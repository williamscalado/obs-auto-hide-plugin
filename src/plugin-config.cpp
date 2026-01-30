#include "plugin-config.hpp"
#include <QJsonArray>
#include <obs-module.h>
#include <QDir>
#include <QFileInfo>
#include <filesystem>

void PluginConfig::save_to_file(const QString &filepath) {
  blog(LOG_INFO, "[Auto Hide] Tentando salvar config em: %s", filepath.toUtf8().constData());

  // Usar std::filesystem para evitar problemas de ABI com QDir::mkpath no macOS
  std::filesystem::path path;
#ifdef _WIN32
  path = std::filesystem::path(filepath.toStdWString());
#else
  QByteArray pathBytes = filepath.toUtf8();
  path = std::filesystem::path(pathBytes.constData());
#endif
  std::filesystem::path dir_path = path.parent_path();

  std::error_code ec;
  if (!std::filesystem::exists(dir_path)) {
      if (std::filesystem::create_directories(dir_path, ec)) {
          blog(LOG_INFO, "[Auto Hide] Diretório criado: %s", dir_path.c_str());
      } else {
          blog(LOG_WARNING, "[Auto Hide] Falha ao criar diretório: %s (Erro: %s)",
               dir_path.c_str(), ec.message().c_str());
      }
  }

  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly)) {
    blog(LOG_WARNING, "[Auto Hide] Erro ao abrir arquivo para escrita: %s",
         filepath.toUtf8().constData());
    return;
  }

  QJsonObject root = to_json();
  QJsonDocument doc(root);
  QByteArray jsonBytes = doc.toJson();

  blog(LOG_INFO, "[Auto Hide] Salvando JSON: %s", jsonBytes.constData());

  qint64 bytes = file.write(jsonBytes);
  file.close();

  blog(LOG_INFO, "[Auto Hide] Config salva com sucesso (%lld bytes)", bytes);
}

void PluginConfig::load_from_file(const QString &filepath) {
  blog(LOG_INFO, "[Auto Hide] Tentando carregar config de: %s", filepath.toUtf8().constData());

  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly)) {
    blog(LOG_INFO, "[Auto Hide] Arquivo de config não encontrado (usando padrões).");
    return;
  }

  QByteArray data = file.readAll();
  file.close();

  blog(LOG_INFO, "[Auto Hide] JSON Lido: %s", data.constData());

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isNull() && doc.isObject()) {
    from_json(doc.object());
    blog(LOG_INFO, "[Auto Hide] Config carregada com sucesso.");
  } else {
    blog(LOG_WARNING, "[Auto Hide] Arquivo de config inválido ou corrompido.");
  }
}

QJsonObject PluginConfig::to_json() const {
  QJsonObject root;
  root["version"] = "1.0.0";

  // Holyrics
  QJsonObject holyrics;
  holyrics["url"] = holyrics_url;
  holyrics["polling_interval"] = polling_interval_ms;
  root["holyrics"] = holyrics;

  // Plugin
  QJsonObject plugin;
  plugin["auto_activate"] = auto_activate;
  plugin["disable_in_music"] = disable_in_music;
  root["plugin"] = plugin;

  // Scenes
  QJsonObject scenes;
  scenes["monitored_scene"] = monitored_scene;
  QJsonArray sources_array;
  for (const QString &s : sources_to_hide) {
    sources_array.append(s);
  }
  scenes["sources_to_hide"] = sources_array;
  root["scenes"] = scenes;

  // Behavior
  QJsonObject behavior;
  behavior["restore_previous_state"] = restore_previous_state;
  behavior["action_delay_ms"] = action_delay_ms;
  behavior["show_notifications"] = show_notifications;
  root["behavior"] = behavior;

  return root;
}

void PluginConfig::from_json(const QJsonObject &json) {
  if (json.contains("holyrics")) {
    QJsonObject holyrics = json["holyrics"].toObject();
    holyrics_url = holyrics["url"].toString(holyrics_url);
    polling_interval_ms =
        holyrics["polling_interval"].toInt(polling_interval_ms);
  }

  if (json.contains("plugin")) {
    QJsonObject plugin = json["plugin"].toObject();
    auto_activate = plugin["auto_activate"].toBool(auto_activate);
    disable_in_music = plugin["disable_in_music"].toBool(disable_in_music);
  }

  if (json.contains("scenes")) {
    QJsonObject scenes = json["scenes"].toObject();
    monitored_scene = scenes["monitored_scene"].toString();

    sources_to_hide.clear();
    QJsonArray sources_array = scenes["sources_to_hide"].toArray();
    for (const auto &val : sources_array) {
      sources_to_hide.append(val.toString());
    }
  }

  if (json.contains("behavior")) {
    QJsonObject behavior = json["behavior"].toObject();
    restore_previous_state =
        behavior["restore_previous_state"].toBool(restore_previous_state);
    action_delay_ms = behavior["action_delay_ms"].toInt(action_delay_ms);
    show_notifications =
        behavior["show_notifications"].toBool(show_notifications);
  }
}
