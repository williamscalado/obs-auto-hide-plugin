#include "auto-hide-dock.hpp"
#include "holyrics-client.hpp"
#include "plugin-config.hpp"
#include "scene-controller.hpp"
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/bmem.h>

OBS_DECLARE_MODULE()
// OBS_MODULE_USE_DEFAULT_LOCALE("auto-hide-scenes", "en-US")

class AutoHidePlugin {
private:
  PluginConfig config;
  HolyricsClient *holyrics_client;
  SceneController *scene_controller;
  AutoHideDockWidget *dock_widget;

public:
  AutoHidePlugin() {
    // Inicializar componentes
    holyrics_client = new HolyricsClient();
    scene_controller = new SceneController();
    dock_widget =
        new AutoHideDockWidget(config, holyrics_client, scene_controller);

    // Configurar callback
    holyrics_client->on_verse_changed = [this](bool visible) {
      on_verse_state_changed(visible);
    };

    holyrics_client->on_deactivation_requested = [this]() {
        if (dock_widget) {
            // Se for música e estiver configurado para desativar:
            // 1. Esconde as fontes (câmera), pois a letra deve estar na tela
            scene_controller->hide_sources(config.sources_to_hide);
            
            // 2. Desativa o plugin SEM restaurar o estado (para manter escondido)
            dock_widget->set_active(false, false);
            
            blog(LOG_INFO, "[Auto Hide] Desativação automática (MUSIC): Fontes ocultadas e plugin parado.");
        }
    };
  }

  ~AutoHidePlugin() {
    delete holyrics_client;
    delete scene_controller;
    // dock_widget é deletado pelo OBS ao fechar ou remover dock
  }

  void on_verse_state_changed(bool verse_visible) {
    blog(LOG_INFO, "[Auto Hide] Versículo: %s",
         verse_visible ? "APARECEU" : "SUMIU");

    if (verse_visible) {
      // Esconder fontes configuradas
      scene_controller->hide_sources(config.sources_to_hide);
    } else {
      // Restaurar estado anterior
      if (config.restore_previous_state) {
        scene_controller->restore_previous_state();
      } else {
        scene_controller->show_all_sources(config.sources_to_hide);
      }
    }

    // Atualizar UI
    dock_widget->update_last_event(verse_visible);
  }

  void load_config() {
    char *path_ptr = obs_module_config_path("config.json");
    if (path_ptr) {
        QString path = QString(path_ptr);
        blog(LOG_INFO, "[Auto Hide] Carregando configuração de: %s", path_ptr);
        config.load_from_file(path);
        bfree(path_ptr);
    } else {
        blog(LOG_WARNING, "[Auto Hide] obs_module_config_path retornou NULL");
    }

    // Aplicar configurações iniciais
    holyrics_client->set_polling_interval(config.polling_interval_ms);
    holyrics_client->set_disable_in_music(config.disable_in_music);
    scene_controller->set_action_delay(config.action_delay_ms);

    // Se ativado automaticamente
    if (config.auto_activate) {
      dock_widget->set_active(true);
    }
  }

  void save_config() {
    char *path_ptr = obs_module_config_path("config.json");
    if (path_ptr) {
        QString path = QString(path_ptr);
        config.save_to_file(path);
        bfree(path_ptr);
    }
  }

  AutoHideDockWidget *get_dock_widget() { return dock_widget; }
};

// Instância global
static AutoHidePlugin *plugin_instance = nullptr;

bool obs_module_load(void) {
  blog(LOG_INFO, "[Auto Hide] Plugin carregado v1.0.0");

  plugin_instance = new AutoHidePlugin();

  // Registrar Dock
  obs_frontend_add_dock_by_id("auto-hide-scenes", "Cidade Viva Plugin", plugin_instance->get_dock_widget());

  // Carregar configuração
  plugin_instance->load_config();

  return true;
}

void obs_module_unload(void) {
  blog(LOG_INFO, "[Auto Hide] Plugin descarregado");

  if (plugin_instance) {
    plugin_instance->save_config();
    delete plugin_instance;
    plugin_instance = nullptr;
  }
}

const char *obs_module_name(void) { return "Auto Hide Scenes"; }

const char *obs_module_description(void) {
  return "Esconde automaticamente fontes quando versículos bíblicos aparecem "
         "no Holyrics";
}
