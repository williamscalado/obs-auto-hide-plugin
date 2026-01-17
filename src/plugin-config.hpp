#pragma once

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QStringList>

struct PluginConfig {
  // Holyrics
  QString holyrics_url = "http://localhost:9000";
  int polling_interval_ms = 1000;

  // Controle
  QString monitored_scene;
  QStringList sources_to_hide;

  // Comportamento
  bool restore_previous_state = true;
  int action_delay_ms = 150;
  bool show_notifications = true;
  bool auto_activate = false; // Padrão: DESLIGADO
  bool disable_in_music = false; // Padrão: DESLIGADO

  // Métodos
  void save_to_file(const QString &filepath);
  void load_from_file(const QString &filepath);
  QJsonObject to_json() const;
  void from_json(const QJsonObject &json);
};
