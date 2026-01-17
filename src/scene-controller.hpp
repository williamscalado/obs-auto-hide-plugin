#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <map>
#include <obs.h>

struct SourceState {
  QString name;
  bool was_visible;
};

class SceneController : public QObject {
  Q_OBJECT

public:
  explicit SceneController(QObject *parent = nullptr);
  virtual ~SceneController();

  // Listar cenas e fontes
  QStringList get_available_scenes();
  QStringList get_scene_sources(const QString &scene_name);

  // Ações principais
  void hide_sources(const QStringList &source_names);
  void restore_previous_state();
  void show_all_sources(const QStringList &source_names); // Fallback

  // Utilitários de visibilidade
  bool is_source_visible(const QString &source_name);
  void set_source_visibility(const QString &source_name, bool visible);

  // Configuração
  void set_action_delay(int ms);

private:
  std::map<QString, SourceState> saved_states;
  int action_delay_ms = 150;

  void save_current_state(const QStringList &source_names);
};
