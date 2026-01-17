#pragma once

#include "holyrics-client.hpp"
#include "plugin-config.hpp"
#include "scene-controller.hpp"
#include <QDockWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class AutoHideDockWidget : public QWidget {
  Q_OBJECT

public:
  explicit AutoHideDockWidget(PluginConfig &config, HolyricsClient *client,
                              SceneController *controller,
                              QWidget *parent = nullptr);
  ~AutoHideDockWidget() override;

  // Métodos de controle
  void set_active(bool active);
  void update_connection_status(bool connected);
  void update_last_event(bool verse_visible);

private slots:
  void on_toggle_clicked();
  void open_settings();

private:
  PluginConfig &config;
  HolyricsClient *holyrics_client;
  SceneController *scene_controller;

  // UI Elements
  QLabel *connection_status_label;
  QPushButton *toggle_button;
  QLabel *status_label;
  QLabel *last_event_label;
  QGroupBox *sources_group;
  QLabel *sources_list_label;
  QPushButton *settings_button;

  bool plugin_active = false;

  void setup_ui();
  void update_ui_state();
  void update_sources_list();
};
