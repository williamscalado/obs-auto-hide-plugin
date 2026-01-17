#pragma once

#include "plugin-config.hpp"
#include "scene-controller.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  explicit SettingsDialog(PluginConfig &config, SceneController *controller,
                          QWidget *parent = nullptr);
  ~SettingsDialog() override;

private slots:
  void save();
  void test_connection();
  void on_scene_changed(const QString &scene_name);
  void add_source_manually();

private:
  PluginConfig &config;
  SceneController *scene_controller;
  QNetworkAccessManager *network_manager;

  // UI Components
  QLineEdit *url_input;
  QSpinBox *interval_input;
  QPushButton *test_button;
  QLabel *status_label;

  QComboBox *scene_combo;
  QListWidget *sources_list;

  QCheckBox *restore_state_check;
  QSpinBox *delay_input;
  QCheckBox *notifications_check;
  QCheckBox *auto_activate_check;
  QCheckBox *disable_in_music_check;

  void setup_ui();
  void load_current_values();
};
