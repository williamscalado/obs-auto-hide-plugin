#pragma once

#include "presentation-client.hpp"
#include "plugin-config.hpp"
#include "scene-controller.hpp"
#include <QDockWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>

class AutoHideDockWidget : public QWidget {
  Q_OBJECT

public:
  std::function<void()> on_settings_changed;
  explicit AutoHideDockWidget(PluginConfig &config, IPresentationClient **client_ptr,
                              SceneController *controller,
                              QWidget *parent = nullptr);
  ~AutoHideDockWidget() override;

  // Métodos de controle
  void set_active(bool active, bool restore_state = true);
  void update_connection_status(bool connected);
  void update_last_event(bool verse_visible);
  
  bool is_active() const { return plugin_active; }
  void update_ui_state();
  void update_sources_list();

private slots:
  void on_toggle_clicked();
  void open_settings();

private:
  PluginConfig &config;
  IPresentationClient **active_client_ptr;
  SceneController *scene_controller;

  // UI Elements
  QLabel *connection_status_label;
  QLabel *client_info_label;
  QPushButton *toggle_button;
  QLabel *status_label;
  QLabel *last_event_label;
  QGroupBox *sources_group;
  QLabel *sources_list_label;
  QPushButton *settings_button;

  bool plugin_active = false;

  void setup_ui();
};
